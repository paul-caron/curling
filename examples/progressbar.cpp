#include <iostream>
#include <iomanip>
#include "curling.hpp"

bool progressCallback(curl_off_t dltotal, curl_off_t dlnow, curl_off_t, curl_off_t) {
    if (!dltotal) return false;
    double p = (double)dlnow / dltotal;
    int w = 50, pos = (int)(p * w);
    std::cout << "\r[";
    for (int i = 0; i < w; ++i) std::cout << (i < pos ? '=' : (i == pos ? '>' : ' '));
    std::cout << "] " << std::fixed << std::setprecision(1) << (p*100) << "% "
              << "(" << dlnow << "/" << dltotal << " bytes)" << std::flush;
    return false;
}

int main() {
    //some heavy file to download amd witness progress bar
    std::string url = "https://testfiles.hostnetworks.com.au/100MB.iso";
    std::string out = "100MB.iso";

    curling::Request req;
    req.setMethod(curling::Request::Method::GET)
       .setURL(url)
       .setFollowRedirects(true)
       .addHeader("Accept: */*")
       .addHeader("User-Agent: Curling/1.0")
       .setProgressCallback(progressCallback)
       .downloadToFile(out)
       .setTimeout(120)
       .enableVerbose(true);

    std::cout << "Downloading " << url << std::endl;
    auto res = req.send();
    std::cout << "\nFinished with HTTP code: " << res.httpCode << std::endl;
}
