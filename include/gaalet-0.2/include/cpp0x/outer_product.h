#ifndef __GAALET_OUTER_PRODUCT_H
#define __GAALET_OUTER_PRODUCT_H

#include "utility.h"

namespace gaalet
{

namespace op
{

//list of element multiplications with the same result type
struct msl_null
{
   static const conf_t size = 0;

   template<typename element_t, class L, class R>
   static element_t product_sum(const L&, const R&)
   {
      return 0.0;
   }
};

template<conf_t LC, conf_t RC, typename T>
struct multiplication_sum_list
{
   static const conf_t left = LC;
   static const conf_t right = RC;

   typedef T tail;

   static const conf_t size = T::size + 1;

   template<typename element_t, class L, class R>
   static element_t product_sum(const L& l, const R& r)
   {
      return
         l.template element<left>()*r.template element<right>()
         *CanonicalReorderingSign<left, right>::value
         + tail::template product_sum<element_t>(l, r);
   }
};
template<conf_t LC, conf_t RC>
struct multiplication_sum_list<LC, RC, msl_null>
{
   static const conf_t left = LC;
   static const conf_t right = RC;

   typedef msl_null tail;

   static const conf_t size = 1;

   template<typename element_t, class L, class R>
   static element_t product_sum(const L& l, const R& r)
   {
      return
         l.template element<left>()*r.template element<right>()
         *CanonicalReorderingSign<left, right>::value;
   }
};

//list of multiplication_sum_list, with respect to result type
template<conf_t C, typename H, typename T>
struct multiplication_element_list
{
   static const conf_t conf = C;
   typedef H head;
   typedef T tail;

   typedef configuration_list<conf, typename T::clist> clist;

   static const conf_t size = T::size + 1;

   template<typename element_t, class L, class R>
   static element_t product_sum(const L& l, const R& r)
   {
      return head::template product_sum<element_t>(l, r);
   }
};

struct mel_null
{
   static const conf_t conf = 0;
   typedef cl_null clist;

   static const conf_t size = 0;

   template<typename element_t, class L, class R>
   static element_t product_sum(const L&, const R&)
   {
      return 0.0;
   }
};

//insert_element to multiplication_element_list
template<conf_t LC, conf_t RC, typename list, int op = (BitCount<LC^RC>::value == (BitCount<LC>::value+BitCount<RC>::value)) ?
                                                   (((LC^RC)==list::conf) ? 0 : (((LC^RC)<list::conf) ? 1 : -1))
                                                   : -10>
struct insert_element_to_melist
{
   typedef multiplication_element_list<list::conf, typename list::head, typename insert_element_to_melist<LC, RC, typename list::tail>::melist> melist;
};

template<conf_t LC, conf_t RC, int op>
struct insert_element_to_melist<LC, RC, mel_null, op>
{
   typedef multiplication_element_list<LC^RC, multiplication_sum_list<LC, RC, msl_null>, mel_null> melist;
};
template<conf_t LC, conf_t RC>
struct insert_element_to_melist<LC, RC, mel_null, 0>
{
   typedef multiplication_element_list<LC^RC, multiplication_sum_list<LC, RC, msl_null>, mel_null> melist;
};

template<conf_t LC, conf_t RC, typename list>
struct insert_element_to_melist<LC, RC, list, 0>
{
   typedef multiplication_element_list<list::conf, multiplication_sum_list<LC, RC, typename list::head>, typename list::tail> melist;
};

template<conf_t LC, conf_t RC, typename list>
struct insert_element_to_melist<LC, RC, list, 1>
{
   typedef multiplication_element_list<LC^RC, multiplication_sum_list<LC, RC, msl_null>, list> melist;
};

template<conf_t LC, conf_t RC, typename list>
struct insert_element_to_melist<LC, RC, list, -10>
{
   typedef list melist;
};
template<conf_t LC, conf_t RC>
struct insert_element_to_melist<LC, RC, mel_null, -10>
{
   typedef mel_null melist;
};

//build_multiplication_element_list
template<typename L, typename R, typename metric, typename CL=L, int op = metric::degenerate_bitmap&(L::head&R::head)>
struct build_multiplication_element_list
{
   typedef typename build_multiplication_element_list<typename L::tail, R, metric, CL>::melist melist;
};
template<typename L, typename R, typename metric, typename CL>
struct build_multiplication_element_list<L, R, metric, CL, 0>
{
   typedef typename insert_element_to_melist<L::head, R::head, typename build_multiplication_element_list<typename L::tail, R, metric, CL>::melist>::melist melist;
};
template<typename R, typename metric, typename CL, int op>
struct build_multiplication_element_list<cl_null, R, metric, CL, op>
{
   typedef typename build_multiplication_element_list<CL, typename R::tail, metric, CL>::melist melist;
};
template<typename R, typename metric, typename CL>
struct build_multiplication_element_list<cl_null, R, metric, CL, 0>
{
   typedef typename build_multiplication_element_list<CL, typename R::tail, metric, CL>::melist melist;
};
template<typename L, typename metric, typename CL, int op>
struct build_multiplication_element_list<L, cl_null, metric, CL, op>
{
   typedef mel_null melist;
};
template<typename L, typename metric, typename CL>
struct build_multiplication_element_list<L, cl_null, metric, CL, 0>
{
   typedef mel_null melist;
};

//search melist
template<conf_t conf, typename list, bool fit = (conf==list::conf)>
struct search_conf_in_melist
{
   typedef typename search_conf_in_melist<conf, typename list::tail>::melist melist;
};

template<conf_t conf, typename list>
struct search_conf_in_melist<conf, list, true>
{
   typedef list melist;
};

template<conf_t conf, bool fit>
struct search_conf_in_melist<conf, mel_null, fit>
{
   typedef mel_null melist;
};
template<conf_t conf>
struct search_conf_in_melist<conf, mel_null, true>
{
   typedef mel_null melist;
};

}  //end namespace op

template<class L, class R>
struct outer_product : public expression<outer_product<L, R>>
{
   typedef typename element_type_combination_traits<typename L::element_t, typename R::element_t>::element_t element_t;
   typedef typename metric_combination_traits<typename L::metric, typename R::metric>::metric metric;

   typedef typename op::build_multiplication_element_list<typename L::clist, typename R::clist, metric>::melist melist;
   typedef typename melist::clist clist;


   outer_product(const L& l_ , const R& r_)
      :  l(l_), r(r_)
   { }

   template<conf_t conf>
   element_t element() const {
      return op::search_conf_in_melist<conf, melist>::melist::template product_sum<element_t>(l, r);
   }

protected:
   const L& l;
   const R& r;
};

} //end namespace gaalet

/// Outer product of two multivectors.
/**
 * Following Hestenes' defintion.
 */
/// \ingroup ga_ops
template <class L, class R> inline
gaalet::outer_product<L, R>
operator^(const gaalet::expression<L>& l, const gaalet::expression<R>& r) {
   return gaalet::outer_product<L, R>(l, r);
}

#endif
