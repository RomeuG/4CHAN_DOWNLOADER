#include <iostream>
#include <cstring>
#include <filesystem>
#include <regex>
#include <memory>

#include <libxml++-3.0/libxml++/libxml++.h>
#include <libxml2/libxml/HTMLparser.h>
#include <curl/curl.h>

#include <boost/algorithm/string.hpp>

#include "json.hpp"

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

struct args_t {
	std::string board;
	std::string thread;
	std::string page;
	std::string media;

	bool catalogue = false;
	bool media_download = false;
};

auto _replace(std::string& str, const std::string_view from, const std::string_view to) -> void
{
	if (from.empty()) {
		return;
	}

	std::size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

static auto curlcb_html(char *data, size_t size, size_t nmemb, std::string *writer_data) -> std::size_t
{
	if (writer_data == nullptr) {
		return 0;
	}

	writer_data->append(data, size * nmemb);

	return size * nmemb;
}

static auto curlcb_img(void *ptr, size_t size, size_t nmemb, void *userdata) -> std::size_t
{
	FILE *stream = (FILE *) userdata;
	if (!stream) {
		std::printf("No file input stream\n");
		return 0;
	}

	return (std::size_t) fwrite((FILE *) ptr, size, nmemb, stream);
}

auto download_json(const char *url) -> std::string
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
		return "";
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
		 return "";
	 }

	 curl_slist_free_all(headers);
	 curl_easy_cleanup(curl_ctx);

	 return buffer;
 }

auto convert_to_xmltree(std::string& buffer, htmlDocPtr *document, xmlNode **root) -> bool
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

auto download_media(std::string& url, std::filesystem::path& path) -> bool
{
	// dirty hack because core is being dumped for
	// no apparent reason if this is not done.
	// stack smashing happens too.
	// probably a problem with std::filesystem::path ?
	std::string file = std::string(path.c_str());

	auto fp = std::fopen(file.c_str(), "wb");
	if (!fp) {
		std::printf("Failed to create file on the disk: %d\n", errno);
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

		curl_easy_cleanup(curl_ctx);
		std::fclose(fp);
		return false;
	}

	auto res_code = 0;
	curl_easy_getinfo(curl_ctx, CURLINFO_RESPONSE_CODE, &res_code);

	if (!((res_code == 200 || res_code == 201))) {
		std::printf("Response code: %d\n", res_code);

		curl_easy_cleanup(curl_ctx);
		std::fclose(fp);

		// remove file because its useless
		std::filesystem::remove(path);

		return false;
	}

	curl_easy_cleanup(curl_ctx);
	std::fclose(fp);

	return true;
}

auto get_post_text(xmlpp::Element *element) -> std::string
{
	std::string post;

	auto sibling = element->get_first_child();

	while (sibling) {
		auto sub_sibling = sibling->get_first_child();

		if (sibling->get_name() == "br") {
			post += "\n";
		}

		if (sibling->get_name() == "text") {
			auto text = reinterpret_cast<xmlpp::TextNode *>(sibling);
			if (text) {
				std::string sanitized = text->get_content();
				_replace(sanitized, "\n", "");
				_replace(sanitized, "\r", "");
				post += sanitized;
			}
		}

		while (sub_sibling) {
			if (sub_sibling->get_name() == "br") {
				post += "\n";
			}

			if (sub_sibling->get_name() == "p") {
				auto paragraph_element = reinterpret_cast<xmlpp::Element *>(sub_sibling);
				if (paragraph_element) {
					auto paragraph_text = reinterpret_cast<xmlpp::TextNode *>(sub_sibling->get_first_child());
					post += paragraph_text->get_content();
				}
			}

			if (sub_sibling->get_name() == "i") {
				auto italic_element = reinterpret_cast<xmlpp::Element *>(sub_sibling);
				if (italic_element) {
					auto italic_text = reinterpret_cast<xmlpp::TextNode *>(sub_sibling->get_first_child());
					post += italic_text->get_content();
				}
			}

			if (sub_sibling->get_name() == "text") {
				auto text = reinterpret_cast<xmlpp::TextNode *>(sub_sibling);
				if (text) {
					std::string sanitized = text->get_content();
					_replace(sanitized, "\n", "");
					_replace(sanitized, "\r", "");
					post += sanitized;
				}
			}

			if (sub_sibling->get_name() == "a") {
				auto link_text = reinterpret_cast<xmlpp::TextNode *>(sub_sibling->get_first_child());
				if (link_text) {
					post += link_text->get_content();
				}

				auto link = reinterpret_cast<xmlpp::Element *>(sub_sibling);
				if (link && link_text) {
					auto href = link->get_attribute_value("href");

					if (href != link_text->get_content()) {
						post += " (" + href + ")";
					}
				}
			}

			if (sub_sibling->get_name() == "span") {
				auto quote = reinterpret_cast<xmlpp::TextNode *>(sub_sibling->get_first_child());
				if (quote) {
					post += quote->get_content();
				}
			}

			sub_sibling = sub_sibling->get_next_sibling();
		}

		sibling = sibling->get_next_sibling();
	}

	// two new lines to separate posts
	post += "\n\n";

	return post;
}

auto get_post_text_from_json(std::string& str) -> std::string
{
	std::string text;

	htmlDocPtr doc = nullptr;
	xmlNode *root = nullptr;

	auto result = convert_to_xmltree(str, &doc, &root);
	if (!result) {
		std::printf("Failed converting into xmltree");
		exit(EXIT_FAILURE);
	}

	auto root_element = std::make_unique<xmlpp::Element>(root);
	auto body = root_element->find("//body");

	if (body.empty()) {
		 return "\n\n";
	 }

	 auto body_element = reinterpret_cast<xmlpp::Element *>(body[0]);
	 text = get_post_text(body_element);

	 xmlFreeDoc(doc);

	 return text;
}

auto get_post_info(nlohmann::json& post, struct args_t& args) -> std::string
{
	std::string info;

	if (!post["name"].empty()) {
		info += post["name"].get<std::string>() + " ";
	} else {
		info += post["trip"].get<std::string>() + " ";
	}

	if (!post["filename"].empty()) {
		info += post["filename"].get<std::string>() + post["ext"].get<std::string>();
		info += " (" + post["w"].dump() + "x" + post["h"].dump() + ") ";
	} else {
		if (args.catalogue) {
			info += "<file deleted> ";
		}
	}

	info += post["now"].get<std::string>() + " ";
	info += post["no"].dump() + "\n";

	if (!post["sub"].empty()) {
		info += post["sub"].get<std::string>();
	} else {
		if (args.catalogue) {
			info += "<empty title>";
		}
	}

	if (args.catalogue) {
		info += " (" + post["replies"].dump() + " Replies, " + post["images"].dump() + " Images)\n";
	}

	if (!post["tim"].empty()) {
		info += "Media: http://i.4cdn.org/" + args.board + "/" + post["tim"].dump() + post["ext"].get<std::string>() + "\n";

		if (args.media_download) {
			auto media_url = "http://i.4cdn.org/" + args.board + "/" + post["tim"].dump() + post["ext"].get<std::string>();

			if (!std::filesystem::exists(args.thread)) {
				if (!std::filesystem::create_directory(args.thread)) {
					std::printf("Error creating directory.\n");
				}
			}

			auto absolute_path = std::filesystem::absolute(args.thread);
			absolute_path.append(post["tim"].dump() + post["ext"].get<std::string>());

			if (!std::filesystem::exists(absolute_path)) {
				download_media(media_url, absolute_path);
			}
		}
	}

	if (!post["com"].empty()) {
		// get string and replace <wbr> with empty string
		auto post_text = post["com"].get<std::string>();
		_replace(post_text, "<wbr>", "");

		info += get_post_text_from_json(post_text);
	} else {
		info += "<empty body>\n\n";
	}

	return info;
}

auto get_thread(nlohmann::json& catalogue_json, struct args_t& args) -> void
{
	for (nlohmann::json& post : catalogue_json["posts"]) {
		std::string thread_info = get_post_info(post, args);
		std::printf("%s\n", thread_info.c_str());
	}
}

auto get_catalogue(nlohmann::json& catalogue_json, struct args_t& args) -> void
{
	for (auto& page : catalogue_json) {
		for (nlohmann::json& thread : page["threads"]) {
			std::string thread_info = get_post_info(thread, args);
			std::printf("%s\n", thread_info.c_str());
		}
	}
}

auto usage(char *bin_name) -> void
{
	std::printf("Usage: ./%s etc", bin_name);
}

auto main(int argc, char **argv) -> int
{
	int copts;
	std::string website;

	struct args_t args;

	while ((copts = getopt(argc, argv, "b:chi:It:")) != -1) {
		switch (copts) {
		case 'b': args.board = optarg;
			break;
		case 'c': args.catalogue = true;
			break;
		case 'h':
			// TODO
			usage(argv[0]);
			break;
		case 'i':
			args.media = optarg;
			break;
		case 'I':
			args.media_download = true;
			break;
		case 't': args.thread = optarg;
			break;
		default: break;
		}
	}

	auto board = Constants::chan_map.find(args.board);
	if (board == Constants::chan_map.cend()) {
		std::printf("Invalid imageboard.\n");
		exit(EXIT_FAILURE);
	} else {
		website = board->second;
	}

	if (args.catalogue) {
		website = "http://a.4cdn.org/" + args.board + "/catalog.json";

		auto buffer = download_json(website.c_str());
		nlohmann::json buffer_json = nlohmann::json::parse(buffer);
		get_catalogue(buffer_json, args);
	}

	if (!args.thread.empty()) {
		website = "http://a.4cdn.org/" + args.board + "/thread/" + args.thread + ".json";

		auto buffer = download_json(website.c_str());
		nlohmann::json buffer_json = nlohmann::json::parse(buffer);
		get_thread(buffer_json, args);
	}

	return EXIT_SUCCESS;
}
