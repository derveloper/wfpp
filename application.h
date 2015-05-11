//
// Created by vileda on 01.05.15.
//

#ifndef WFPP_APPLICATION_H
#define WFPP_APPLICATION_H

#include <vector>
#include <functional>

#include <boost/any.hpp>
#include <map>
#include <iostream>
#include <functional>

#include "ext/json.hpp"

#include "lwan.h"
#include "lwan-serve-files.h"
#include "lwan-template.h"


namespace wfpp {
    using json = nlohmann::json;
    using view_data = json;
    using view_object = json;

    struct response {
        lwan_response_t *lwan_response;

        response(lwan_response_t *r) : lwan_response(r) { };
    };

    struct view : view_object {
        response _response;

        view(response &resp) : _response(resp) { };
        lwan_http_status_t render(const view_object &obj, const lwan_http_status_t &status = HTTP_OK) {

            _response.lwan_response->mime_type = "application/json";

            auto render = obj.dump(2);
            strbuf_set_static(_response.lwan_response->buffer, render.c_str(), render.length());

            return status;
        };
    };

    struct application {
        std::vector<lwan_url_map_t> url_map;
        lwan_t l;

        application() {
            lwan_init(&l);
        }

        template<typename handler_fn>
        void get(const char *route, handler_fn &&handler) {
            thread_local handler_fn _h = std::move(handler);
            using lwan_handler_t = lwan_http_status_t (*)(lwan_request_t *, lwan_response_t *, void *);
            lwan_handler_t lwan_handler = [](lwan_request_t *req, lwan_response_t *res, void *data)
                    -> lwan_http_status_t {
                return _h(req, std::move(response(res)), data);
            };
            url_map.push_back((lwan_url_map_t) {.prefix = (char *) route, .handler = lwan_handler});
        }

        constexpr void run() {
            url_map.push_back(lwan_url_map_t {.prefix = (char*)"/", SERVE_FILES((char*)"./wwwroot")});
            url_map.push_back(lwan_url_map_t {.prefix = NULL});
            lwan_set_url_map(&l, url_map.data());
            lwan_main_loop(&l);
        }

        ~application() {
            lwan_shutdown(&l);
        }
    };

};

#endif //WFPP_APPLICATION_H
