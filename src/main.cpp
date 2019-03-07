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

enum {
	NODE_NAME = 0,
	NODE_PATH,
	NODE_VALUE,
};

std::vector<std::string> split_str(const std::string &string, const char delimiter)
{
	std::vector<std::string> results;
	boost::split(results, string, [delimiter](char c){return c == delimiter;});

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

std::string download_html(char *url)
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

void download_imgs(std::vector<std::string> &list)
{
	for (std::string &url : list) {
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
	std::string buffer = download_html(argv[1]);

	// testing html parsing
	htmlDocPtr doc;
	xmlNode *root_element = nullptr;

	doc = htmlReadMemory(buffer.c_str(), buffer.size(), nullptr, nullptr, HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET);
	if (doc == nullptr) {
		std::printf("Error parsing html.\n");
		exit(EXIT_FAILURE);
	}

	root_element = xmlDocGetRootElement(doc);
	if (root_element == nullptr) {
		std::printf("Error getting root element.\n");
		xmlFreeDoc(doc);
		exit(EXIT_FAILURE);
	}

	auto root = new xmlpp::Element(root_element);

	//auto elements = root->find(XPATH_ALL_IMGS);
	auto elements = root->find(XPATH_IMG_THUMB);
	auto node_info = get_node_info<xmlpp::Element *>(elements[0]);

	xmlNode *html_body = htmlparse_get_body(root_element);
	//traverse_dom_trees(html_body);

	xmlFreeDoc(doc);
	return EXIT_SUCCESS;
}
