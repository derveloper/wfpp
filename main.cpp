#include <iostream>

#include "application.h"

constexpr void add_foo(std::vector<int> &bar, int v) {
    bar.push_back(v);
}

int
main(int argc, char **argv)
{
    std::vector<int> bar;
    add_foo(bar, 2);
    add_foo(bar, 4);
    add_foo(bar, 6);
    wfpp::application app;

    app.get("/bar", [] (lwan_request_t *request,
                        lwan_response_t *resp, void *data) {
        std::cout << "req " << std::string(request->original_url.value) << std::endl;
        wfpp::view view{resp};
        wfpp::view_object object{"bar", {
                {"foo", "bar"},
                {"foo2", "bar2"},
                {"foo3", {"bar2", "bar", 2}}
        }};
        return view.render("foo", object);
    });

    app.run();

    return 0;
}