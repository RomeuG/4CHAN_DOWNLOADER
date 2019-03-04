#include <iostream>
#include <cstring>

#include <libxml++-3.0/libxml++/libxml++.h>
#include <libxml2/libxml/HTMLparser.h>
#include <curl/curl.h>

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

std::vector<std::string> split_string(const std::string& s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;

	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))	{
		tokens.push_back(token);
	}

	return tokens;
}

bool download_img(const char *url)
{
	auto fp = std::fopen("out.jpg", "wb");
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
	fclose(fp);

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

static bool init(CURL *& conn, char *url, std::string& buffer)
{
	char curl_error_buffer[CURL_ERROR_SIZE];

	conn = curl_easy_init();

	if (conn == nullptr) {
		std::printf("Failed to create CURL connection\n");
		return false;
	}

	curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, curl_error_buffer);
	curl_easy_setopt(conn, CURLOPT_URL, url);
	curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, curlcb_html);
	curl_easy_setopt(conn, CURLOPT_WRITEDATA, &buffer);

	return true;
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
	auto dump = xmlNodeDump(_buffer, nullptr, element, 2, 1);
	auto result = strdup((char *) xmlBufferContent(_buffer));
	std::printf("Result: %s\n", result);
}

int main(int argc, char **argv)
{
	CURL *conn = nullptr;
	CURLcode code;

	std::string buffer;
	char curl_error_buffer[CURL_ERROR_SIZE];

	if (argc != 2) {
		std::printf("Usage: %s <url>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	curl_global_init(CURL_GLOBAL_DEFAULT);

	if (!init(conn, argv[1], buffer)) {
		std::printf("Connection initialization failed\n");
		exit(EXIT_FAILURE);
	}

	code = curl_easy_perform(conn);
	curl_easy_cleanup(conn);

	if (code != CURLE_OK) {
		std::printf("Failed to get '%s' [%s]\n", argv[1], curl_error_buffer);
		exit(EXIT_FAILURE);
	}

	//std::cout << buffer;
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

	auto elements = root->find("//img");
	auto img_link = reinterpret_cast<xmlpp::Element *>(elements[0])->get_attribute_value("src");
	std::printf("Link: %s", img_link.c_str());

	// for (auto& element : elements) {
	// 	auto link = std::string(reinterpret_cast<xmlpp::Element *>(element)->get_attribute_value("src"));
	// 	std::printf("Image link: %s\n", link.c_str());
	// }

	//print_xml(root_element);

	xmlNode *html_body = htmlparse_get_body(root_element);
	//traverse_dom_trees(html_body);

	auto a = split_string(img_link, '/');
	for (std::string &str : a) { std::printf("%s\n", str.c_str()); }

	xmlFreeDoc(doc);
	return EXIT_SUCCESS;
}
