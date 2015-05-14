#include <iostream>

#include "application.h"

#include <boost/filesystem.hpp>
#include <lwan.h>


namespace fs = boost::filesystem;

static wfpp::view_object file_object{};

int
main(void)
{
    wfpp::application app;

    app.get("/api/files", [] (lwan_request_t *request __attribute__((unused)),
                                   wfpp::response response, void *data __attribute__((unused))) -> lwan_http_status_t {
        wfpp::view view{response};

        auto snd_path = "./wwwroot/soundfiles";

        if(file_object.empty() || (request->query_params.base != nullptr && std::string("refresh").compare(request->query_params.base->key) == 0)) {
            file_object = wfpp::view_object{{"files", std::vector<std::string>()}};

            auto dir_iter = fs::recursive_directory_iterator(fs::path(snd_path));

            while(dir_iter != fs::recursive_directory_iterator{}) {
                fs::directory_entry &f = *dir_iter++;
                file_object["files"].push_back(f.path().string());
            }
        }

        return view.render(file_object);
    });

    app.run();

    return 0;
}
