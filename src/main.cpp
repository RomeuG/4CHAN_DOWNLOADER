#include <iostream>
#include <cstring>

#include <libxml++-3.0/libxml++/libxml++.h>
#include <libxml2/libxml/HTMLparser.h>
#include <curl/curl.h>

#include <boost/algorithm/string.hpp>

// XPATH constants
#define XPATH_ALL_IMGS "//img"
#define XPATH_TITLE "//title"
#define XPATH_IMG_THUMB "//a/img"
#define XPATH_A_CLASS_THUMB "//a[@class='fileThumb']"
#define XPATH_TITLE_AND_IMGS "//title | //a/img"

namespace Constants {
	const std::unordered_map<std::string, std::string> a{
			{"3",    "https://www.boards.4channel.org/3/"},
			{"a",    "https://www.boards.4channel.org/a/"},
			{"an",   "https://www.boards.4channel.org/an/"},
			{"aco",  "https://www.boards.4chan.org/aco/"},
			{"adv",  "https://www.boards.4channel.org/adv/"},
			{"asp",  "https://www.boards.4channel.org/asp/"},
			{"b",    "https://www.boards.4chan.org/b/"},
			{"bant", "https://www.boards.4chan.org/bant/"},
			{"biz",  "https://www.boards.4channel.org/biz/"},
			{"c",    "https://www.boards.4channel.org/c/"},
			{"cgl",  "https://www.boards.4channel.org/cgl/"},
			{"ck",   "https://www.boards.4channel.org/ck/"},
			{"cm",   "https://www.boards.4channel.org/cm/"},
			{"co",   "https://www.boards.4channel.org/co/"},
			{"d",    "https://www.boards.4chan.org/d/"},
			{"diy",  "https://www.boards.4channel.org/diy/"},
			{"e",    "https://www.boards.4chan.org/e/"},
			{"f",    "https://www.boards.4chan.org/f/"},
			{"fa",   "https://www.boards.4channel.org/fa/"},
			{"fit",  "https://www.boards.4channel.org/fit/"},
			{"g",    "https://www.boards.4channel.org/g/"},
			{"gd",   "https://www.boards.4channel.org/gd/"},
			{"gif",  "https://www.boards.4chan.org/gif/"},
			{"h",    "https://www.boards.4chan.org/h/"},
			{"hc",   "https://www.boards.4chan.org/hc/"},
			{"hm",   "https://www.boards.4chan.org/hm/"},
			{"hr",   "https://www.boards.4chan.org/hr/"},
			{"his",  "https://www.boards.4channel.org/his/"},
			{"i",    "https://www.boards.4chan.org/i/"},
			{"ic",   "https://www.boards.4chan.org/ic/"},
			{"int",  "https://www.boards.4channel.org/int/"},
			{"jp",   "https://www.boards.4channel.org/jp/"},
			{"k",    "https://www.boards.4channel.org/k/"},
			{"lgbt", "https://www.boards.4channel.org/lgbt/"},
			{"lit",  "https://www.boards.4channel.org/lit/"},
			{"m",    "https://www.boards.4channel.org/m/"},
			{"mu",   "https://www.boards.4channel.org/mu/"},
			{"mlp",  "https://www.boards.4channel.org/mlp/"},
			{"n",    "https://www.boards.4channel.org/n/"},
			{"news", "https://www.boards.4channel.org/news/"},
			{"o",    "https://www.boards.4channel.org/o/"},
			{"out",  "https://www.boards.4channel.org/out/"},
			{"p",    "https://www.boards.4channel.org/p/"},
			{"po",   "https://www.boards.4channel.org/po/"},
			{"pol",  "https://www.boards.4chan.org/pol/"},
			{"qst",  "https://www.boards.4channel.org/qst/"},
			{"r",    "https://www.boards.4chan.org/r/"},
			{"r9k",  "https://www.boards.4chan.org/r9k/"},
			{"s",    "https://www.boards.4chan.org/s/"},
			{"sp",   "https://www.boards.4channel.org/sp/"},
			{"s4s",  "https://www.boards.4chan.org/s4s/"},
			{"sci",  "https://www.boards.4channel.org/sci/"},
			{"soc",  "https://www.boards.4chan.org/soc/"},
			{"t",    "https://www.boards.4chan.org/t/"},
			{"tg",   "https://www.boards.4channel.org/tg/"},
			{"toy",  "https://www.boards.4channel.org/toy/"},
			{"trv",  "https://www.boards.4channel.org/trv/"},
			{"u",    "https://www.boards.4chan.org/u/"},
			{"v",    "https://www.boards.4channel.org/v/"},
			{"vg",   "https://www.boards.4channel.org/vg/"},
			{"vp",   "https://www.boards.4channel.org/vp/"},
			{"vr",   "https://www.boards.4channel.org/vr/"},
			{"vip",  "https://www.boards.4channel.org/vip/"},
			{"w",    "https://www.boards.4channel.org/w/"},
			{"wg",   "https://www.boards.4chan.org/wg/"},
			{"wsg",  "https://www.boards.4channel.org/wsg/"},
			{"wsr",  "https://www.boards.4channel.org/wsr/"},
			{"x",    "https://www.boards.4channel.org/x/"},
			{"y",    "https://www.boards.4chan.org/y/"},
	};
}

enum
{
	NODE_NAME = 0,
	NODE_PATH,
	NODE_VALUE,
};

std::vector<std::string> split_str(const std::string& string, const char delimiter)
{
	std::vector<std::string> results;
	boost::split(results, string, [delimiter](char c) { return c == delimiter; });

	return results;
}

static std::size_t curlcb_html(char *data, size_t size, size_t nmemb, std::string *writer_data)
{
	if (writer_data == nullptr) {
		return 0;
	}

	writer_data->append(data, size * nmemb);

	return size * nmemb;
}

static size_t curlcb_img(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	FILE *stream = (FILE *) userdata;
	if (!stream) {
		std::printf("No file input stream\n");
		return 0;
	}

	return (std::size_t) fwrite((FILE *) ptr, size, nmemb, stream);
}

std::string download_html(const char *url)
{
	CURL *conn = nullptr;
	CURLcode code;

	std::string buffer;
	char curl_error_buffer[CURL_ERROR_SIZE];

	curl_global_init(CURL_GLOBAL_DEFAULT);

	conn = curl_easy_init();

	if (conn == nullptr) {
		std::printf("Failed to create CURL connection\n");
		return nullptr;
	}

	curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, curl_error_buffer);
	curl_easy_setopt(conn, CURLOPT_URL, url);
	curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, curlcb_html);
	curl_easy_setopt(conn, CURLOPT_WRITEDATA, &buffer);

	code = curl_easy_perform(conn);
	curl_easy_cleanup(conn);

	if (code != CURLE_OK) {
		std::printf("Failed to get '%s' [%s]\n", url, curl_error_buffer);
		exit(EXIT_FAILURE);
	}

	return buffer;
}

bool convert_to_xmltree(std::string buffer, htmlDocPtr *document, xmlNode **root)
{
	*document = htmlReadMemory(buffer.c_str(), buffer.size(), nullptr, nullptr, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
	if (document == nullptr) {
		std::printf("Error parsing html.\n");
		return false;
	}

	*root = xmlDocGetRootElement(*document);
	if (*root == nullptr) {
		std::printf("Error getting root element.\n");
		xmlFreeDoc(*document);
		return false;
	}

	return true;
}

bool download_img(const char *url)
{
	auto file_name = split_str(url, '/').back();
	auto fp = std::fopen(file_name.c_str(), "wb");

	if (!fp) {
		std::printf("Failed to create file on the disk\n");
		return false;
	}

	auto curl_ctx = curl_easy_init();

	curl_easy_setopt(curl_ctx, CURLOPT_URL, url);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEFUNCTION, curlcb_img);
	curl_easy_setopt(curl_ctx, CURLOPT_FOLLOWLOCATION, 1);

	CURLcode rc = curl_easy_perform(curl_ctx);
	if (rc) {
		std::printf("Failed to download: %s\n", url);
		return false;
	}

	auto res_code = 0;
	curl_easy_getinfo(curl_ctx, CURLINFO_RESPONSE_CODE, &res_code);

	if (!((res_code == 200 || res_code == 201))) {
		std::printf("Response code: %d\n", res_code);
		return false;
	}

	curl_easy_cleanup(curl_ctx);
	std::fclose(fp);

	return true;
}

void download_imgs(std::vector<std::string>& list)
{
	for (std::string& url : list) {
		auto res = download_img(url.c_str());
		if (!res) {
			std::printf("Error downloading image from %s\n", url.c_str());
		}
	}
}

void traverse_dom_trees(xmlNode *a_node)
{
	xmlNode *cur_node = nullptr;

	if (a_node == nullptr) {
		return;
	}

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			std::printf("Node type: Text, name: %s\n", cur_node->name);
		} else if (cur_node->type == XML_TEXT_NODE) {
			std::printf("node type: Text, node content: %s,  content length %zu\n", (char *) cur_node->content, strlen((char *) cur_node->content));
		}

		traverse_dom_trees(cur_node->children);
	}
}

xmlNode *htmlparse_get_body(xmlNode *root_node)
{
	xmlNode *possible_body = root_node->children->next;
	if (strncmp((char *) possible_body->name, "body", 5) == 0) {
		return possible_body;
	}

	return nullptr;
}

void print_xml(xmlNode *element)
{
	xmlBufferPtr _buffer = xmlBufferCreate();
	xmlNodeDump(_buffer, nullptr, element, 2, 1);
	auto result = strdup((char *) xmlBufferContent(_buffer));
	std::printf("Result: %s\n", result);
	xmlBufferFree(_buffer);
}

template<class T>
std::array<std::string, 3> get_node_info(xmlpp::Node *node)
{
	std::array<std::string, 3> node_info;
	auto node_vector = reinterpret_cast<T>(node);

	node_info[NODE_NAME] = node_vector->get_name();
	node_info[NODE_PATH] = node_vector->get_path();
	node_info[NODE_VALUE] = node_vector->get_first_child_text()->get_content();

	return node_info;
}

int main(int argc, char **argv)
{
	int copts;

	std::string board;
	std::string thread;
	std::string index_page;

	htmlDocPtr doc = nullptr;
	xmlNode *root = nullptr;

	while ((copts = getopt(argc, argv, "b:chp:t:")) != -1) {
		switch (copts) {
		case 'b':
			board = optarg;
			break;
		case 'c':
			// TODO: download catalog with first post information
			break;
		case 'h':
			// TODO
			std::printf("Usage: ./program etc");
			break;
		case 'p':
			index_page = optarg;
			break;
		case 't':
			thread = optarg;
			break;
		default: break;
		}
	}

	std::string website = std::string("https://boards.4chan.org/") + std::string(board);
	auto buffer = download_html(website.c_str());
	auto result = convert_to_xmltree(buffer, &doc, &root);
	if (!result) {
		std::printf("Something went terribly wrong.\n");
		exit(EXIT_FAILURE);
	}

	 auto root_element = new xmlpp::Element(root);

	 //auto elements = root->find(XPATH_ALL_IMGS);
	 auto elements = root_element->find("//img/preceding::a[1]");
	 for (auto& element : elements) {
		 auto e = reinterpret_cast<xmlpp::Element *>(element);
		 std::printf("Element tag: %s\n", e->get_attribute("href")->get_value().c_str());
	 }
	 //auto node_info = get_node_info<xmlpp::Element *>(elements[0]);

	 xmlNode *html_body = htmlparse_get_body(root);
	//traverse_dom_trees(html_body);

	xmlFreeDoc(doc);
	return EXIT_SUCCESS;
}
