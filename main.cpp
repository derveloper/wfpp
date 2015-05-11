#include <iostream>

#include "application.h"

int
main(void)
{
    wfpp::application app;

    app.get("/api/bar", [] (lwan_request_t *request __attribute__((unused)),
                                   wfpp::response response, void *data __attribute__((unused))) -> lwan_http_status_t {
        wfpp::view view{response};
        wfpp::view_object object{"bar2", {
                {"foo", "bar"},
                {"foo2", "bar2"},
                {"foo3", {"bar2", "bar", 2}}
        }};
        return view.render(object);
    });

    app.run();

    return 0;
}