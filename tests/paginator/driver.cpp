#include <crails/paginator.hpp>
#include <any>
#include <string>

// Assertions must stay active whatever the build configuration (ie: -DNDEBUG).
#undef NDEBUG
#include <cassert>

// Assertions wrapped in `#ifdef PATCHED` describe the behavior we expect from
// the paginator, but which it does not honor yet (see the comments next to each
// of them). Build with `config.cxx.poptions += -DPATCHED` to enable them.

using namespace Crails;
using namespace std;

// Request parameters always reach the paginator as strings.
static DataTree make_params(const char* page = nullptr, const char* count = nullptr)
{
  DataTree params;

  if (page)  params["page"]  = string(page);
  if (count) params["count"] = string(count);
  return params;
}

template<typename T>
static T var(const SharedVars& vars, const char* name)
{
  return any_cast<T>(vars.find(name)->second);
}

int main()
{
  // Defaults: first page, default count, no offset
  {
    DataTree params = make_params();
    Paginator subject(params.as_data());

    assert(subject.is_enabled());
    assert(subject.get_current_page() == 1);
    assert(subject.get_items_per_page() == CRAILS_PAGINATOR_DEFAULT_COUNT);
    assert(subject.get_current_offset() == 0);
  }

  // Reads the page and count parameters
  {
    DataTree params = make_params("3", "20");
    Paginator subject(params.as_data());

    assert(subject.get_current_page() == 3);
    assert(subject.get_items_per_page() == 20);
  }

  // The offset is the number of items skipped by the previous pages
  {
    DataTree first = make_params("1", "20");
    DataTree second = make_params("2", "20");
    DataTree fifth = make_params("5", "7");

    assert(Paginator(first.as_data()).get_current_offset() == 0);
    assert(Paginator(second.as_data()).get_current_offset() == 20);
    assert(Paginator(fifth.as_data()).get_current_offset() == 28);
  }

  // The page parameter alone uses the default count
  {
    DataTree params = make_params("3");
    Paginator subject(params.as_data());

    assert(subject.get_items_per_page() == CRAILS_PAGINATOR_DEFAULT_COUNT);
    assert(subject.get_current_offset() == 2 * CRAILS_PAGINATOR_DEFAULT_COUNT);
  }

  // The count parameter alone starts at the first page
  {
    DataTree params = make_params(nullptr, "20");
    Paginator subject(params.as_data());

    assert(subject.get_current_page() == 1);
    assert(subject.get_items_per_page() == 20);
    assert(subject.get_current_offset() == 0);
  }

  // Parameters may be integers rather than strings (ie: set programmatically)
  {
    DataTree params;
    params["page"] = 2;
    params["count"] = 50;
    Paginator subject(params.as_data());

    assert(subject.get_current_page() == 2);
    assert(subject.get_items_per_page() == 50);
    assert(subject.get_current_offset() == 50);
  }

  // The count is capped by the maximum number of items per page
  {
    DataTree params = make_params("2", "5000");
    Paginator subject(params.as_data());

    assert(subject.get_items_per_page() == CRAILS_PAGINATOR_MAX_ITEMS);
    assert(subject.get_current_offset() == CRAILS_PAGINATOR_MAX_ITEMS);
  }

  // A count equal to the maximum is accepted as is
  {
    DataTree params = make_params(nullptr, "300");
    Paginator subject(params.as_data());

    assert(subject.get_items_per_page() == 300);
  }

  // The maximum can be lowered...
  {
    DataTree params = make_params(nullptr, "50");
    Paginator subject(params.as_data());

    subject.set_max_items_per_page(10);
    assert(subject.get_items_per_page() == 10);
  }

  // ...or raised
  {
    DataTree params = make_params(nullptr, "500");
    Paginator subject(params.as_data());

    assert(subject.get_items_per_page() == CRAILS_PAGINATOR_MAX_ITEMS);
    subject.set_max_items_per_page(1000);
    assert(subject.get_items_per_page() == 500);
  }

  // The default count can be changed
  {
    DataTree params = make_params();
    Paginator subject(params.as_data());

    subject.set_default_items_per_page(25);
    assert(subject.get_items_per_page() == 25);
    assert(subject.get_current_offset() == 0);
  }

  // An explicit count wins over the default one
  {
    DataTree params = make_params(nullptr, "5");
    Paginator subject(params.as_data());

    subject.set_default_items_per_page(25);
    assert(subject.get_items_per_page() == 5);
  }

  // The default count is subject to the maximum as well
  {
    DataTree params = make_params();
    Paginator subject(params.as_data());

    subject.set_default_items_per_page(1000);
    assert(subject.get_items_per_page() == CRAILS_PAGINATOR_MAX_ITEMS);
    subject.set_max_items_per_page(40);
    assert(subject.get_items_per_page() == 40);
  }

  // The paginator reads its parameters when asked, not when constructed
  {
    DataTree params = make_params();
    Paginator subject(params.as_data());

    assert(subject.get_current_page() == 1);
    params["page"] = string("4");
    params["count"] = string("10");
    assert(subject.get_current_page() == 4);
    assert(subject.get_items_per_page() == 10);
    assert(subject.get_current_offset() == 30);
  }

  // Parameters which are not numbers are ignored
  {
    DataTree params = make_params("abc", "xyz");
    Paginator subject(params.as_data());

    assert(subject.get_current_page() == 1);
    assert(subject.get_items_per_page() == CRAILS_PAGINATOR_DEFAULT_COUNT);
    assert(subject.get_current_offset() == 0);
  }

  // So are parameters which are only partially numbers, or empty
  {
    DataTree params = make_params("2x", "5x");
    DataTree empty = make_params("", "");
    Paginator subject(params.as_data());
    Paginator blank(empty.as_data());

    assert(subject.get_current_page() == 1);
    assert(subject.get_items_per_page() == CRAILS_PAGINATOR_DEFAULT_COUNT);
    assert(blank.get_current_page() == 1);
    assert(blank.get_items_per_page() == CRAILS_PAGINATOR_DEFAULT_COUNT);
  }

  // Only the paginator's own parameters matter
  {
    DataTree params = make_params("2", "10");
    params["id"] = string("12");
    params["controller-data"]["action"] = string("index");
    Paginator subject(params.as_data());

    assert(subject.get_current_page() == 2);
    assert(subject.get_items_per_page() == 10);
  }

  // Paginators are copyable, copies share the request's parameters
  {
    DataTree params = make_params("2", "10");
    Paginator subject(params.as_data());
    Paginator copy(subject);

    copy.set_max_items_per_page(5);
    assert(copy.get_items_per_page() == 5);
    assert(subject.get_items_per_page() == 10);
    assert(copy.get_current_page() == 2);
  }

  // ---- decorate_view

  // Exposes the pagination state to the views
  {
    DataTree params = make_params("3", "20");
    Paginator subject(params.as_data());
    SharedVars vars;

    subject.decorate_view(vars, []() -> unsigned long { return 123; });
    assert(var<bool>(vars, "with_paginator") == true);
    assert(var<unsigned int>(vars, "page") == 3);
    assert(var<unsigned int>(vars, "items_per_page") == 20);
    assert(var<unsigned long>(vars, "items_count") == 123);
    assert(vars.size() == 4);
  }

  // Decorates the view with the capped count, not the requested one
  {
    DataTree params = make_params("1", "5000");
    Paginator subject(params.as_data());
    SharedVars vars;

    subject.decorate_view(vars, []() -> unsigned long { return 0; });
    assert(var<unsigned int>(vars, "items_per_page") == CRAILS_PAGINATOR_MAX_ITEMS);
  }

  // The total is only computed once
  {
    DataTree params = make_params();
    Paginator subject(params.as_data());
    SharedVars vars;
    int calls = 0;

    subject.decorate_view(vars, [&calls]() -> unsigned long { ++calls; return 5; });
    assert(calls == 1);
  }

  // A disabled paginator tells the views, and does not ask for the total
  {
    DataTree params = make_params("3", "20");
    Paginator subject(params.as_data());
    SharedVars vars;
    int calls = 0;

    subject.set_enabled(false);
    assert(!subject.is_enabled());
    subject.decorate_view(vars, [&calls]() -> unsigned long { ++calls; return 5; });
    assert(calls == 0);
    assert(var<bool>(vars, "with_paginator") == false);
    assert(vars.size() == 1);
  }

  // Can be enabled again
  {
    DataTree params = make_params();
    Paginator subject(params.as_data());
    SharedVars vars;

    subject.set_enabled(false);
    subject.set_enabled(true);
    subject.decorate_view(vars, []() -> unsigned long { return 7; });
    assert(var<bool>(vars, "with_paginator") == true);
    assert(var<unsigned long>(vars, "items_count") == 7);
  }

  // Leaves unrelated variables alone
  {
    DataTree params = make_params();
    Paginator subject(params.as_data());
    SharedVars vars;

    vars["title"] = string("Articles");
    subject.decorate_view(vars, []() -> unsigned long { return 1; });
    assert(any_cast<string>(vars["title"]) == "Articles");
  }

  // ---- Behaviors which are surprising at best, and expected to change:

  // `page=0` asks for a page which does not exist. The offset should not
  // underflow: today, `0 - 1` wraps around and the offset is ~4 billion.
  {
    DataTree params = make_params("0", "20");
    Paginator subject(params.as_data());

#ifdef PATCHED
    assert(subject.get_current_page() == 1);
    assert(subject.get_current_offset() == 0);
#endif
  }

  // Negative pages are parsed as unsigned: `page=-1` turns into page 65535
  {
    DataTree params = make_params("-1", "20");
    Paginator subject(params.as_data());

#ifdef PATCHED
    assert(subject.get_current_page() == 1);
    assert(subject.get_current_offset() == 0);
#endif
  }

  // A count of zero produces a `LIMIT 0` and a division by zero for any view
  // computing the number of pages (items_count / items_per_page).
  {
    DataTree params = make_params("1", "0");
    Paginator subject(params.as_data());

#ifdef PATCHED
    assert(subject.get_items_per_page() >= 1);
#endif
  }

  // Negative counts are parsed as unsigned: `count=-5` turns into the maximum
  {
    DataTree params = make_params("1", "-5");
    Paginator subject(params.as_data());

#ifdef PATCHED
    assert(subject.get_items_per_page() == CRAILS_PAGINATOR_DEFAULT_COUNT);
#endif
  }

  // Parameters are parsed as `unsigned short`: values above 65535 are not
  // parsed, and silently replaced by the defaults.
  {
    DataTree params = make_params("70000", "70000");
    Paginator subject(params.as_data());

    subject.set_max_items_per_page(100000);
#ifdef PATCHED
    assert(subject.get_current_page() == 70000);
    assert(subject.get_items_per_page() == 70000);
#endif
  }

  return 0;
}
