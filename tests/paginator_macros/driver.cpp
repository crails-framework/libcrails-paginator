// The limits are configurable at compile time. Both macros must be defined
// before the first inclusion of the header, which is why this lives in its own
// driver.
#define CRAILS_PAGINATOR_MAX_ITEMS     50
#define CRAILS_PAGINATOR_DEFAULT_COUNT 20

#include <crails/paginator.hpp>
#include <string>

#undef NDEBUG
#include <cassert>

using namespace Crails;
using namespace std;

int main()
{
  // The default count comes from CRAILS_PAGINATOR_DEFAULT_COUNT
  {
    DataTree params;
    Paginator subject(params.as_data());

    assert(subject.get_items_per_page() == 20);
    assert(subject.get_current_offset() == 0);
  }

  // The offset follows the configured default
  {
    DataTree params;
    Paginator subject(params.as_data());

    params["page"] = string("3");
    assert(subject.get_current_offset() == 40);
  }

  // The maximum comes from CRAILS_PAGINATOR_MAX_ITEMS
  {
    DataTree params;
    Paginator subject(params.as_data());

    params["count"] = string("1000");
    assert(subject.get_items_per_page() == 50);
    params["count"] = string("50");
    assert(subject.get_items_per_page() == 50);
    params["count"] = string("49");
    assert(subject.get_items_per_page() == 49);
  }

  // Both can still be overridden at runtime
  {
    DataTree params;
    Paginator subject(params.as_data());

    subject.set_default_items_per_page(5);
    assert(subject.get_items_per_page() == 5);
    subject.set_max_items_per_page(1);
    assert(subject.get_items_per_page() == 1);
  }

  return 0;
}
