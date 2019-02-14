#include <iostream>

#include <curl/curl.h>

static int writer(char *data, size_t size, size_t nmemb, std::string *writer_data)
{
	if(writer_data == nullptr) {
		return 0;
	}

	writer_data->append(data, size*nmemb);

	return size * nmemb;
}

static bool init(CURL *&conn, char *url, std::string &buffer)
{
	CURLcode code;
	char curl_error_buffer[CURL_ERROR_SIZE];

	conn = curl_easy_init();

	if (conn == nullptr) {
		std::printf("Failed to create CURL connection\n");
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, curl_error_buffer);
	if (code != CURLE_OK) {
		std::printf("Failed to set error buffer [%d]\n", code);
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_URL, url);
	if (code != CURLE_OK) {
		std::printf("Failed to set URL [%s]\n", curl_error_buffer);
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1L);
	if (code != CURLE_OK) {
		std::printf("Failed to set redirect option [%s]\n", curl_error_buffer);
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writer);
	if (code != CURLE_OK) {
		std::printf("Failed to set writer [%s]\n", curl_error_buffer);
		return false;
	}

	code = curl_easy_setopt(conn, CURLOPT_WRITEDATA, &buffer);
	if (code != CURLE_OK) {
		std::printf("Failed to set write data [%s]\n", curl_error_buffer);
		return false;
	}

	return true;
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

	std::cout << buffer;

	return EXIT_SUCCESS;
}
