#include "server_info.h"

bool Server_Info::IsServerOnline(std::string_view url)
{
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    curl_easy_setopt(curl, CURLOPT_URL, url.data());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);        // HEAD request
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1L);       // overall timeout
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1L); // connection timeout

    // Disable all retries
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L); // no redirect following
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 0L);      // max redirects = 0
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);    // fail on HTTP errors

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}