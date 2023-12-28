#ifndef  PAGINATOR_HPP
# define PAGINATOR_HPP

# ifdef WITH_ODB
#  include <odb/query.hxx>
# endif
# ifdef WITH_MONGODB
#  include <mongocxx/pipeline.hpp>
# endif
# include <crails/datatree.hpp>
# include <crails/shared_vars.hpp>

# ifndef CRAILS_PAGINATOR_MAX_ITEMS
#  define CRAILS_PAGINATOR_MAX_ITEMS 300
# endif

# ifndef CRAILS_PAGINATOR_DEFAULT_COUNT
#  define CRAILS_PAGINATOR_DEFAULT_COUNT 100
# endif

namespace Crails
{
  class Paginator
  {
  public:
    typedef std::function<unsigned long()> CountCallback;

    enum Options
    {
      NoOptions = 0,
      UseDefaultOrderBy = 1
    };

    Paginator(Data params) : params(params)
    {
    }

    void set_enabled(bool enabled) { this->enabled = enabled; }
    bool is_enabled() const { return enabled; }
    void set_max_items_per_page(unsigned int value) { max_items_per_page = value; }
    void set_default_items_per_page(unsigned int value) { default_items_per_page = value; }
    unsigned int get_items_per_page() const;
    unsigned int get_current_offset() const;
    unsigned int get_current_page() const;
    void decorate_view(Crails::SharedVars& vars, CountCallback get_total_items) const;

# ifdef WITH_ODB
    template<typename MODEL>
    void decorate_query(odb::query<MODEL>& query, char options = UseDefaultOrderBy) const
    {
      if (enabled)
      {
        if ((options & UseDefaultOrderBy) > 0)
          query = MODEL::default_order_by(query);
        query = query
          + "LIMIT"  + odb::query<MODEL>::_val(get_items_per_page())
          + "OFFSET" + odb::query<MODEL>::_val(get_current_offset());
      }
    }
# endif

# ifdef WITH_MONGODB
    void decorate_query(mongocxx::pipeline& pipe) const
    {
      pipe.skip(get_current_offset());
      pipe.limit(get_items_per_page());
    }
# endif

  private:
    bool enabled = true;
    Data params;
    unsigned int max_items_per_page = CRAILS_PAGINATOR_MAX_ITEMS;
    unsigned int default_items_per_page = CRAILS_PAGINATOR_DEFAULT_COUNT;
  };
}

#endif
