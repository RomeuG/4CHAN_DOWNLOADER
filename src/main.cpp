#include <iostream>
#include <cstring>
#include <filesystem>

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
#define XPATH_A_BEFORE_IMG "//img/preceding::a[1]"

#define XPATH_BLOCKQUOTE_TEXT "//blockquote[@class='postMessage']/text()"

#define XPATH_OP_POST "//div[@class='post op']"
#define XPATH_REPLY_POST "//div[@class='post reply']"

//auto replies = root->find("//blockquote[@class='postMessage']/text()");
//auto replies = root->find("//blockquote[@class='postMessage']");

namespace Constants {
	const std::unordered_map<std::string, std::string> chan_map{
			{"3",    "https://boards.4channel.org/3/"},
			{"a",    "https://boards.4channel.org/a/"},
			{"an",   "https://boards.4channel.org/an/"},
			{"aco",  "https://boards.4chan.org/aco/"},
			{"adv",  "https://boards.4channel.org/adv/"},
			{"asp",  "https://boards.4channel.org/asp/"},
			{"b",    "https://boards.4chan.org/b/"},
			{"bant", "https://boards.4chan.org/bant/"},
			{"biz",  "https://boards.4channel.org/biz/"},
			{"c",    "https://boards.4channel.org/c/"},
			{"cgl",  "https://boards.4channel.org/cgl/"},
			{"ck",   "https://boards.4channel.org/ck/"},
			{"cm",   "https://boards.4channel.org/cm/"},
			{"co",   "https://boards.4channel.org/co/"},
			{"d",    "https://boards.4chan.org/d/"},
			{"diy",  "https://boards.4channel.org/diy/"},
			{"e",    "https://boards.4chan.org/e/"},
			{"f",    "https://boards.4chan.org/f/"},
			{"fa",   "https://boards.4channel.org/fa/"},
			{"fit",  "https://boards.4channel.org/fit/"},
			{"g",    "https://boards.4channel.org/g/"},
			{"gd",   "https://boards.4channel.org/gd/"},
			{"gif",  "https://boards.4chan.org/gif/"},
			{"h",    "https://boards.4chan.org/h/"},
			{"hc",   "https://boards.4chan.org/hc/"},
			{"hm",   "https://boards.4chan.org/hm/"},
			{"hr",   "https://boards.4chan.org/hr/"},
			{"his",  "https://boards.4channel.org/his/"},
			{"i",    "https://boards.4chan.org/i/"},
			{"ic",   "https://boards.4chan.org/ic/"},
			{"int",  "https://boards.4channel.org/int/"},
			{"jp",   "https://boards.4channel.org/jp/"},
			{"k",    "https://boards.4channel.org/k/"},
			{"lgbt", "https://boards.4channel.org/lgbt/"},
			{"lit",  "https://boards.4channel.org/lit/"},
			{"m",    "https://boards.4channel.org/m/"},
			{"mu",   "https://boards.4channel.org/mu/"},
			{"mlp",  "https://boards.4channel.org/mlp/"},
			{"n",    "https://boards.4channel.org/n/"},
			{"news", "https://boards.4channel.org/news/"},
			{"o",    "https://boards.4channel.org/o/"},
			{"out",  "https://boards.4channel.org/out/"},
			{"p",    "https://boards.4channel.org/p/"},
			{"po",   "https://boards.4channel.org/po/"},
			{"pol",  "https://boards.4chan.org/pol/"},
			{"qst",  "https://boards.4channel.org/qst/"},
			{"r",    "https://boards.4chan.org/r/"},
			{"r9k",  "https://boards.4chan.org/r9k/"},
			{"s",    "https://boards.4chan.org/s/"},
			{"sp",   "https://boards.4channel.org/sp/"},
			{"s4s",  "https://boards.4chan.org/s4s/"},
			{"sci",  "https://boards.4channel.org/sci/"},
			{"soc",  "https://boards.4chan.org/soc/"},
			{"t",    "https://boards.4chan.org/t/"},
			{"tg",   "https://boards.4channel.org/tg/"},
			{"toy",  "https://boards.4channel.org/toy/"},
			{"trv",  "https://boards.4channel.org/trv/"},
			{"u",    "https://boards.4chan.org/u/"},
			{"v",    "https://boards.4channel.org/v/"},
			{"vg",   "https://boards.4channel.org/vg/"},
			{"vp",   "https://boards.4channel.org/vp/"},
			{"vr",   "https://boards.4channel.org/vr/"},
			{"vip",  "https://boards.4channel.org/vip/"},
			{"w",    "https://boards.4channel.org/w/"},
			{"wg",   "https://boards.4chan.org/wg/"},
			{"wsg",  "https://boards.4channel.org/wsg/"},
			{"wsr",  "https://boards.4channel.org/wsr/"},
			{"x",    "https://boards.4channel.org/x/"},
			{"y",    "https://boards.4chan.org/y/"},
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
	CURL *curl_ctx = nullptr;
	CURLcode code;

	struct curl_slist *headers = nullptr;

	std::string buffer;
	char curl_error_buffer[CURL_ERROR_SIZE];

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl_ctx = curl_easy_init();

	if (curl_ctx == nullptr) {
		std::printf("Failed to create CURL connection\n");
		return nullptr;
	}

	headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3724.8 Safari/537.36");

	curl_easy_setopt(curl_ctx, CURLOPT_ERRORBUFFER, curl_error_buffer);
	curl_easy_setopt(curl_ctx, CURLOPT_URL, url);
	curl_easy_setopt(curl_ctx, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl_ctx, CURLOPT_FOLLOWLOCATION, 0L);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEFUNCTION, curlcb_html);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEDATA, &buffer);

	code = curl_easy_perform(curl_ctx);

	if (code != CURLE_OK) {
		std::printf("Failed to get '%s' [%s]\n", url, curl_error_buffer);
		exit(EXIT_FAILURE);
	}

	auto res_code = 0;
	curl_easy_getinfo(curl_ctx, CURLINFO_RESPONSE_CODE, &res_code);

	if (!((res_code == 200 || res_code == 201 || res_code == 403))) {
		std::printf("Response code: %d\n", res_code);
		return nullptr;
	}

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl_ctx);

	return buffer;
}

bool convert_to_xmltree(std::string buffer, htmlDocPtr *document, xmlNode **root)
{
	*document = htmlReadMemory(buffer.c_str(), buffer.size(), nullptr, nullptr, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
	if (*document == nullptr) {
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

bool remove_file(std::string& path)
{
	return std::filesystem::remove(path);
}

bool download_img(Glib::ustring& _url)
{
	auto url = std::string(_url);

	// erase leading //
	if (url[0] == '/' && url[1] == '/') {
		url.erase(0, 2);
	}

	// get url for the original sized image
	auto s = url.find_last_of('s');
	if (s != std::string::npos) {
		url.erase(url.begin() + s);
	}

	auto file_name = split_str(url, '/').back();
	auto file_type = split_str(url, '.').back();

	auto fp = std::fopen(file_name.c_str(), "wb");
	if (!fp) {
		std::printf("Failed to create file on the disk\n");
		return false;
	}

	auto curl_ctx = curl_easy_init();

	curl_easy_setopt(curl_ctx, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEFUNCTION, curlcb_img);
	curl_easy_setopt(curl_ctx, CURLOPT_FOLLOWLOCATION, 1);

	CURLcode rc = curl_easy_perform(curl_ctx);
	if (rc) {
		std::printf("Failed to download: %s\n", url.c_str());

		std::fclose(fp);
		return false;
	}

	auto res_code = 0;
	curl_easy_getinfo(curl_ctx, CURLINFO_RESPONSE_CODE, &res_code);

	if (!((res_code == 200 || res_code == 201))) {
		std::printf("Response code: %d\n", res_code);

		curl_easy_cleanup(curl_ctx);
		std::fclose(fp);

		if (file_type != "gif" && file_type == "jpg") {
			std::printf("Trying to fetch the png...\n");

			// delete version of file that is not downloadable
			remove_file(file_name);

			// change url from jpg to png
			auto period = url.find_last_of('.');
			url.replace(period + 1, url.size() - 1, "png");

			Glib::ustring new_url = Glib::ustring(url);
			download_img(new_url);
		}

		// delete if the file ultimately isnt downloadble
		remove_file(file_name);

		return false;
	}

	curl_easy_cleanup(curl_ctx);
	std::fclose(fp);

	return true;
}

bool download_img_thumb(Glib::ustring& url)
{
	auto file_name = split_str(url, '/').back();
	auto file_type = split_str(url, '.').back();

	auto fp = std::fopen(file_name.c_str(), "wb");

	// erase leading //
	if (url[0] == '/' && url[1] == '/') {
		url.erase(0, 2);
	}

	if (!fp) {
		std::printf("Failed to create file on the disk\n");
		return false;
	}

	auto curl_ctx = curl_easy_init();

	curl_easy_setopt(curl_ctx, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl_ctx, CURLOPT_WRITEFUNCTION, curlcb_img);
	curl_easy_setopt(curl_ctx, CURLOPT_FOLLOWLOCATION, 1);

	CURLcode rc = curl_easy_perform(curl_ctx);
	if (rc) {
		std::printf("Failed to download: %s\n", url.c_str());
		return false;
	}

	auto res_code = 0;
	curl_easy_getinfo(curl_ctx, CURLINFO_RESPONSE_CODE, &res_code);

	if (!((res_code == 200 || res_code == 201 || res_code == 403))) {
		std::printf("Response code: %d\n", res_code);
		return false;
	}

	curl_easy_cleanup(curl_ctx);
	std::fclose(fp);

	return true;
}

// TODO receive NODE* as input instead of string
void download_imgs(std::vector<std::string>& list)
{
	for (std::string& url : list) {
		//auto res = download_img(url);
		auto res = true;
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

xmlpp::Element* get_post_header_ptr(xmlpp::Element *element)
{
	auto children = element->get_children();
	for (xmlpp::Node* &child : children) {
		auto e = reinterpret_cast<xmlpp::Element*>(child);
		if (e->get_attribute("class")->get_value() == "postInfo desktop") {
			return e;
		}
	}

	return nullptr;
}

std::string get_post_header(xmlpp::Element *element)
{
	std::string header;
	auto header_element = get_post_header_ptr(element);
	auto header_children = header_element->get_children();

	std::for_each(header_children.begin(), header_children.end(), [&header](xmlpp::Node *child) {
		auto child_element = reinterpret_cast<xmlpp::Element*>(child);

		if (child_element->get_attribute_value("class") == "subject") {
			auto subject_element = reinterpret_cast<xmlpp::TextNode*>(child_element->get_first_child());

			if (subject_element) {
				header += subject_element->get_content() + " ";
			}
		}

		if (child_element->get_attribute_value("class") == "dateTime") {
			auto datetime_element = reinterpret_cast<xmlpp::TextNode*>(child_element->get_first_child());

			if (datetime_element) {
				header += datetime_element->get_content() + " ";
			}
		}

		if (child_element->get_attribute_value("class") == "postNum desktop") {
			auto postnum_children = child_element->get_children();
			auto postnum_child = std::next(postnum_children.begin(), 1);

			auto postnum_element = reinterpret_cast<xmlpp::Element*>(*postnum_child);
			auto postnum = reinterpret_cast<xmlpp::TextNode*>(postnum_element->get_first_child());

			if (postnum) {
				header += postnum->get_content();
			}
		}
	});

	return header;
}

std::string get_post_text(xmlpp::Element *element)
{
	std::string post;

	auto blockquote = reinterpret_cast<xmlpp::Element *>(element->get_children().back());

	auto sibling = blockquote->get_first_child();
	while (sibling) {
		if (sibling->get_name() == "br") {
			post += "\n";
		}

		if (sibling->get_name() == "text") {
			auto text = reinterpret_cast<xmlpp::TextNode *>(sibling);
			if (text) {
				post += text->get_content();
			}
		}

		if (sibling->get_name() == "a") {
			auto link_text = reinterpret_cast<xmlpp::TextNode*>(sibling->get_first_child());
			if (link_text) {
				post += link_text->get_content();
			}
		}

		if (sibling->get_name() == "span") {
			auto quote = reinterpret_cast<xmlpp::TextNode*>(sibling->get_first_child());
			if (quote) {
				post += quote->get_content();
			}
		}

		sibling = sibling->get_next_sibling();
	}

	// two new lines to separate posts
	post += "\n\n";

	return post;
}

void get_thread(xmlpp::Element *root)
{
	auto op = root->find(XPATH_OP_POST);
	auto replies = root->find(XPATH_REPLY_POST);

	std::for_each(op.begin(), op.end(), [](xmlpp::Node *element) {
		auto post = reinterpret_cast<xmlpp::Element *>(element);

		auto header = get_post_header(post);
		auto text = get_post_text(post);
		std::printf("%s\n", header.c_str());
		std::printf("%s\n", text.c_str());
	});

	std::for_each(replies.begin(), replies.end(), [](xmlpp::Node *element) {
		auto post = reinterpret_cast<xmlpp::Element *>(element);

		auto header = get_post_header(post);
		auto text = get_post_text(post);
		std::printf("%s\n", header.c_str());
		std::printf("%s\n", text.c_str());
	});
}

int main(int argc, char **argv)
{
	int copts;

	std::string arg_board;
	std::string arg_thread;
	std::string arg_page;

	htmlDocPtr doc = nullptr;
	xmlNode *root = nullptr;

	while ((copts = getopt(argc, argv, "b:chp:t:")) != -1) {
		switch (copts) {
			case 'b': arg_board = optarg;
				break;
			case 'c':
				// TODO: download catalog with first post information
				break;
			case 'h':
				// TODO
				std::printf("Usage: ./program etc");
				break;
			case 'p': arg_page = optarg;
				break;
			case 't': arg_thread = optarg;
				break;
			default: break;
		}
	}

	std::string website;
	auto board = Constants::chan_map.find(arg_board);
	if (board == Constants::chan_map.cend()) {
		std::printf("Invalid imageboard.\n");
		exit(EXIT_FAILURE);
	} else {
		website = board->second;
	}

	if (!arg_thread.empty()) {
		website = website + "thread/" + arg_thread;
	}

	auto buffer = download_html(website.c_str());
	auto result = convert_to_xmltree(buffer, &doc, &root);
	if (!result) {
		std::printf("Something went terribly wrong.\n");
		exit(EXIT_FAILURE);
	}

	auto root_element = new xmlpp::Element(root);
	get_thread(root_element);

	xmlNode *html_body = htmlparse_get_body(root);
	//traverse_dom_trees(html_body);

	delete root_element;
	xmlFreeDoc(doc);

	return EXIT_SUCCESS;
}
