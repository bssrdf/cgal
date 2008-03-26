// TODO: Add licence
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL:$
// $Id: $
// 
//
// Author(s)     : Eric Berberich <eric@mpi-inf.mpg.de>
//                 Pavel Emeliyanenko <asm@mpi-sb.mpg.de>
//
// ============================================================================

/*! \file Algebraic_curve_kernel_2.h
 *  \brief defines class \c Algebraic_curve_kernel_2
 *  
 *  Curve and curve pair analysis for algebraic plane curves
 */

#ifndef CGAL_ALGEBRAIC_CURVE_KERNEL_2_H
#define CGAL_ALGEBRAIC_CURVE_KERNEL_2_H

#include <CGAL/basic.h>
#include <CGAL/Algebraic_kernel_1.h>

#include <CGAL/Algebraic_curve_kernel_2/LRU_hashed_map.h>
#include <CGAL/Algebraic_curve_kernel_2/Xy_coordinate_2.h>
#include <CGAL/Algebraic_curve_kernel_2/Algebraic_real_traits.h>
#include <CGAL/Algebraic_curve_kernel_2/Curve_analysis_2.h>
#include <CGAL/Algebraic_curve_kernel_2/Curve_pair_analysis_2.h>

CGAL_BEGIN_NAMESPACE

template < class AlgebraicCurvePair_2, class AlgebraicKernel_1 >
class Algebraic_curve_kernel_2 {

// for each predicate functor defines a member function returning an instance
// of this predicate
#define CGAL_Algebraic_Kernel_pred(Y,Z) \
    Y Z() const { return Y(); }

// the same for construction functors
#define CGAL_Algebraic_Kernel_cons(Y,Z) CGAL_Algebraic_Kernel_pred(Y,Z)

protected:    
    // temporary types
    
    // type of an internal curve pair
    typedef AlgebraicCurvePair_2 Internal_curve_pair_2;

    // type of an internal curve
    typedef typename AlgebraicCurvePair_2::Algebraic_curve_2 Internal_curve_2;

public:
    //!\name public typedefs
    //!@{
    
    //! type of internal x_coordinate
    typedef typename Internal_curve_2::X_coordinate X_coordinate_1;
    
    //! type of internal coefficient
    typedef typename Internal_curve_2::Coefficient Coefficient;

    //! type of internal polynomial
    typedef typename Internal_curve_2::Poly_d Internal_polynomial_2;
    
    typedef typename NiX::Polynomial_traits<Internal_polynomial_2>::
        Innermost_coefficient Innermost_coefficient;
   
    //! type of 1D algebraic kernel
    typedef AlgebraicKernel_1 Algebraic_kernel_1;

    //! myself
    typedef Algebraic_curve_kernel_2<AlgebraicCurvePair_2, AlgebraicKernel_1>
        Self;
    
    // TODO remove when deriving from AK_1
    typedef typename Algebraic_kernel_1::Boundary Boundary;
        
    //! new CGAL univariate polynomial type (_CGAL postfix is temporary to
    //! avoid type clashes with \c Polynomial_2 type defined later
    typedef ::CGAL::Polynomial<Innermost_coefficient> Polynomial_1_CGAL;
    
    //! new CGAL bivariate polynomial type
    typedef ::CGAL::Polynomial<Polynomial_1_CGAL> Polynomial_2_CGAL;
    
    //! bivariate polynomial traits
    typedef ::CGAL::Polynomial_traits_d< Polynomial_2_CGAL >
        Polynomial_traits_2;

    //! type of a curve point
    typedef CGALi::Xy_coordinate_2<Self> Xy_coordinate_2;

    //! type of 1-curve analysis
    typedef CGALi::Curve_analysis_2<Self> Curve_analysis_2; 

    //! type of 2-curve analysis
    typedef CGALi::Curve_pair_analysis_2<Self> Curve_pair_analysis_2;

    //! berfriending representations to make protected typedefs available
    friend class CGALi::Curve_analysis_2_rep<Self>;
    friend class CGALi::Curve_pair_analysis_2_rep<Self>;
    
    //!@}
protected:
    //! \name private functors
    //!@{
    
    //! temporary functor providing conversion from \c Poly_in type to
    //! \c Poly_out type, required for NumeriX <-> CGAL polynomial type
    //! conversion
    template <class Poly_2_from, class Poly_2_to>
    struct Polynomial_converter 
    {
        typedef typename Poly_2_from::NT Poly_1_from;
        typedef typename Poly_2_to::NT Poly_1_to;
        // needed for make_transform_iterator
        typedef Poly_1_to result_type;
        
        Poly_2_to operator()(const Poly_2_from& p) const
        {
            return Poly_2_to(
                ::boost::make_transform_iterator(p.begin(), *this),
                ::boost::make_transform_iterator(p.end(), *this));
        }
        Poly_1_to operator()(const Poly_1_from& p) const
        {
            return Poly_1_to(p.begin(), p.end());
        }
    };
    
    //! polynomial canonicalizer: temporarily we use NiX functors since
    //! \c Poly is NiX-type polynomial
    template <class Poly> 
    struct Poly_canonicalizer : public Unary_function< Poly, Poly >
    {
    // use Polynomial_traits_d<>::Canonicalize ?
        Poly operator()(Poly p) 
        {
            typedef CGAL::Scalar_factor_traits<Poly> Sf_traits;
            typedef typename Sf_traits::Scalar Scalar;
            typename Sf_traits::Scalar_factor scalar_factor;
            typename Sf_traits::Scalar_div scalar_div;
            Scalar g = scalar_factor(p);
            if (g == Scalar(0)) {
                     CGAL_assertion(p == Poly(Scalar(0)));
                     return p;
            }
            CGAL_assertion(g != Scalar(0));
            if(g != Scalar(1)) 
                scalar_div(p,g);
            if(p.lcoeff().lcoeff() < 0) 
                scalar_div(p,Scalar(-1));
            return p;        
        }
           
    };

    // to remove a confusion with Curve_pair_2
    typedef std::pair<Curve_analysis_2, Curve_analysis_2> Pair_of_curves_2;
    
    //! orders pair items by ids
    struct Pair_id_order {

        template<class T1, class T2>
        std::pair<T1, T2> operator()(const std::pair<T1, T2>& p) const {
            if(p.first.id() > p.second.id())
                return std::make_pair(p.second, p.first);
            return p;
        }
    };
    
    //! polynomial pair gcd creator
    template <class Poly> 
    struct Poly_pair_gcd_creator {
    
        typedef std::pair<Poly, Poly> Poly_pair;
        typedef Poly_pair argument_type;
        typedef Poly result_type;
            
        Poly operator()(const Poly_pair& p) const
        {
            return NiX::gcd(p.first, p.second);
        }
    };     

    template <class Result>
    struct Pair_creator {

        template<class T1, class T2>
        Result operator()(const std::pair<T1, T2>& p) const {
            return Result(p.first, p.second);
        }
    };
    
    struct Pair_id_equal_to {

        template <class T1, class T2>
        bool operator()(const std::pair<T1, T2>& p1,
                const std::pair<T1, T2>& p2) const {
            return (p1.first.id() == p2.first.id() &&
                 p1.second.id() == p2.second.id());
        }
    };

    //! type of curve analysis cache
    typedef CGALi::LRU_hashed_map<Internal_polynomial_2,
        Curve_analysis_2, CGALi::Poly_hasher,
        std::equal_to<Internal_polynomial_2>,
        Poly_canonicalizer<Internal_polynomial_2> > Curve_cache;

        //CGALi::Curve_pair_hasher_2<Curve_analysis_2>,
    //! type of curve pair analysis cache 
    typedef CGALi::LRU_hashed_map<Pair_of_curves_2,
        Curve_pair_analysis_2, CGALi::Pair_hasher, Pair_id_equal_to,
        Pair_id_order,
        Pair_creator<Curve_pair_analysis_2> > Curve_pair_cache;


    typedef CGALi::LRU_hashed_map<std::pair<
        typename Self::Xy_coordinate_2,
        typename Self::Xy_coordinate_2>,
        CGAL::Comparison_result, CGALi::Pair_hasher,
        Pair_id_equal_to > Cmp_xy_map;
      
    
    typedef std::pair<Internal_polynomial_2, Internal_polynomial_2>
        Pair_of_internal_polynomial_2;

    template<typename T> struct Gcd {
    
        T operator() (std::pair<T,T> pair) {
            return NiX::gcd(pair.first,pair.second);
        }
    } ;     


    template<typename T> struct Pair_cannonicalize {
    
        std::pair<T,T> operator() (std::pair<T,T> pair) {
        
            if(pair.first > pair.second) 
                return std::make_pair(pair.second,pair.first);
            return pair;
        }
    };

    typedef CGAL::Pair_lexicographical_less_than
        <Internal_polynomial_2, Internal_polynomial_2,
            std::less<Internal_polynomial_2>,
            std::less<Internal_polynomial_2> > Internal_polynomial_2_compare;
    
    typedef CGAL::Cache<Pair_of_internal_polynomial_2,
                        Internal_polynomial_2,
                        Gcd<Internal_polynomial_2>,
                        Pair_cannonicalize<Internal_polynomial_2>,
                        Internal_polynomial_2_compare> Gcd_cache_2;

    //!@}
protected:
    //!\name protected methods and data types
    //!@{
                        
    static Gcd_cache_2& gcd_cache_2() {
        static Gcd_cache_2 cache;
        return cache;
    }

    //!@}
public:
    //!\name cache access functions
    //!@{
    //! access to the static curve cache
    static Curve_cache& curve_cache() 
    {
        static Curve_cache _m_curve_cache;
        return _m_curve_cache;
    }
    
    //! access to the static curve pair cache
    static Curve_pair_cache& curve_pair_cache() 
    {
        static Curve_pair_cache _m_curve_pair_cache;
        return _m_curve_pair_cache;
    }
    
    //!@}
    //! \name public functors and predicates
    //!@{
       
    //! NumeriX to CGAL polynomial type conversion
    typedef Polynomial_converter<Internal_polynomial_2, Polynomial_2_CGAL>
                NiX2CGAL_converter;
    //! CGAL to NumeriX polynomial type conversion
    typedef Polynomial_converter<Polynomial_2_CGAL, Internal_polynomial_2>
                CGAL2NiX_converter;
                
    //! \brief default constructor
    Algebraic_curve_kernel_2() //: _m_curve_cache() 
    {  }
    
    /*! \brief
     * constructs \c Curve_analysis_2 from bivariate polynomial, uses caching
     * when appropriate
     */
    struct Construct_curve_2 :
        public Unary_function< Internal_polynomial_2, Curve_analysis_2 > {
            
        Curve_analysis_2 operator()
                (const Internal_polynomial_2& f) const {
            return Self::curve_cache()(f);
        }
        Curve_analysis_2 operator()
                (const Polynomial_2_CGAL& f) const {
            CGAL2NiX_converter cvt;
            return Self::curve_cache()(cvt(f));
        }
    };
    CGAL_Algebraic_Kernel_cons(Construct_curve_2, construct_curve_2_object);

    /*! \brief
     * constructs \c Curve_pair_analysis_2 from pair of 1-curve analysis,
     * caching is used when appropriate
     */
    struct Construct_curve_pair_2 :
            public Binary_function<Curve_analysis_2, Curve_analysis_2,
                Curve_pair_analysis_2> {
           
        Curve_pair_analysis_2 operator()
           (const Curve_analysis_2& ca1, const Curve_analysis_2& ca2) const {
                 
            Curve_pair_analysis_2 cpa_2 =
                Self::curve_pair_cache()(std::make_pair(ca1, ca2));
            cpa_2._set_swapped(ca1.id() > ca2.id());
            return cpa_2;

            /*Pair_of_curves_2 poc(ca1, ca2);
            bool swap = (ca1.id() > ca2.id());
            if(swap) {
                poc.first = ca2;
                poc.second = ca1;
            }   
            std::pair<typename Curve_pair_cache::Hashed_iterator, bool> res =
                Self::curve_pair_cache().find(poc);
            if(res.second) {
                Curve_pair_analysis_2 cpa = res.first->second;
                cpa._set_swapped(swap);
                return cpa;
            }

            Curve_pair_analysis_2 cpa_2(poc.first, poc.second);
            cpa_2._set_swapped(swap);
            return (Self::curve_pair_cache().
                insert(std::make_pair(poc, cpa_2))).first->second;*/
        }
    };
    CGAL_Algebraic_Kernel_cons(Construct_curve_pair_2,
        construct_curve_pair_2_object);
    
    //! traits class for \c X_coordinate
    typedef CGALi::Algebraic_real_traits<X_coordinate_1> X_real_traits_1;

    //! traits class for \c Xy_coorinate_2
    typedef CGALi::Algebraic_real_traits_for_y<Xy_coordinate_2,
         Internal_curve_pair_2> Y_real_traits_1;

        
 mutable Cmp_xy_map _m_cmp_xy;   
    
    //! returns the first coordinate of \c Xy_coordinate_2
    struct Get_x_2 :
        public Unary_function<Xy_coordinate_2, X_coordinate_1> {
        
        X_coordinate_1 operator()(const Xy_coordinate_2& xy) const {
            return xy.x();
        }
    };
    CGAL_Algebraic_Kernel_cons(Get_x_2, Get_x_2_object);
    
    //! returns the second coordinate of \c Xy_coordinate_2
    struct Get_y_2 :
        public Unary_function<Xy_coordinate_2, X_coordinate_1> {
        
        X_coordinate_1 operator()(const Xy_coordinate_2& xy) const {
            return xy.y();
        }
    };
    CGAL_Algebraic_Kernel_cons(Get_y_2, Get_y_2_object);
    
    struct Refine_x_2 :
        public Unary_function<Xy_coordinate_2, void> {
      
        //! \brief returns at least half's the current interval of the first
        //! coordinate of \c r
        //!
        //! note that an interval may also degenerate to a single point
        void operator()(const Xy_coordinate_2& r) const {
            r.refine_x();            
        }
        
        //! \brief refines the current interval of the first coordinate of \c r
        //! w.r.t. given relative precision
        //!
        //! that is:
        //! <tt>|lower - upper|/|r.x()| <= 2^(-rel_prec)</tt> 
        void operator()(Xy_coordinate_2& r, int rel_prec) const {  
            r.refine_x(rel_prec);
        }
    };
    CGAL_Algebraic_Kernel_pred(Refine_x_2, refine_x_2_object);
    
    struct Refine_y_2 :
        public Unary_function<Xy_coordinate_2, void> {
      
        //! \brief returns at least half's the current interval of the second
        //! coordinate of \c r
        //!
        //! note that an interval may also degenerate to a single point
        void operator()(const Xy_coordinate_2& r) const {
            typename Y_real_traits_1::Refine()(r);
        }
        
        //! \brief refines the current interval of the second coordinate of 
        //! \c r w.r.t. given relative precision
        //!
        //! that is:
        //! <tt>|lower - upper|/|r.y()| <= 2^(-rel_prec)</tt> 
        void operator()(Xy_coordinate_2& r, int rel_prec) const {  
            typename Y_real_traits_1::Refine()(r, rel_prec);
        }
    };
    CGAL_Algebraic_Kernel_pred(Refine_y_2, refine_y_2_object);
    
    //! computes the current lower boundary of the first coordinate of \c r
    struct Lower_boundary_x_2 {
       
        typedef Xy_coordinate_2 agrument_type;
        typedef typename Algebraic_kernel_1::Boundary result_type;
            
        result_type operator()(const Xy_coordinate_2& r) {
            return typename X_real_traits_1::Lower_boundary()(r.x());
        }
    };
    CGAL_Algebraic_Kernel_cons(Lower_boundary_x_2, lower_boundary_x_2_object);
    
    //! computes the current upper boundary of the first coordinate of \c r
    struct Upper_boundary_x_2 {
       
        typedef Xy_coordinate_2 agrument_type;
        typedef typename Algebraic_kernel_1::Boundary result_type;
            
        result_type operator()(const Xy_coordinate_2& r) {
            return typename X_real_traits_1::Upper_boundary()(r.x());
        }
    };
    CGAL_Algebraic_Kernel_cons(Upper_boundary_x_2, upper_boundary_x_2_object);
    
    //! computes the current lower boundary of the second coordinate of \c r
    struct Lower_boundary_y_2 {
       
        typedef Xy_coordinate_2 agrument_type;
        typedef typename Algebraic_kernel_1::Boundary result_type;
            
        result_type operator()(const Xy_coordinate_2& r) {
            return typename Y_real_traits_1::Lower_boundary()(r);
        }
    };
    CGAL_Algebraic_Kernel_cons(Lower_boundary_y_2, lower_boundary_y_2_object);
    
    //! computes the current lower boundary of the second coordinate of \c r
    struct Upper_boundary_y_2 {
       
        typedef Xy_coordinate_2 agrument_type;
        typedef typename Algebraic_kernel_1::Boundary result_type;
            
        result_type operator()(const Xy_coordinate_2& r) {
            return typename Y_real_traits_1::Upper_boundary()(r);
        }
    };
    CGAL_Algebraic_Kernel_cons(Upper_boundary_y_2, upper_boundary_y_2_object);
    
    //! returns the number of boundary type in-between x-coordinates of two
    //! Xy_coordinate_2 objects
    struct Boundary_between_x_2 {
       
        typedef Xy_coordinate_2 first_agrument_type;
        typedef Xy_coordinate_2 second_agrument_type;
        typedef typename Algebraic_kernel_1::Boundary result_type;
            
        result_type operator()(const Xy_coordinate_2& r1, 
                const Xy_coordinate_2& r2) const {
            return typename X_real_traits_1::Boundary_between()
                (r1.x(), r2.x());
        }
    };
    CGAL_Algebraic_Kernel_cons(Boundary_between_x_2, 
            boundary_between_x_2_object);
            
    //! returns the number of boundary type in-between y-coordinates of two
    //! Xy_coordinate_2 objects
    struct Boundary_between_y_2 {
       
        typedef Xy_coordinate_2 first_agrument_type;
        typedef Xy_coordinate_2 second_agrument_type;
        typedef typename Algebraic_kernel_1::Boundary result_type;
            
        result_type operator()(const Xy_coordinate_2& r1, 
                const Xy_coordinate_2& r2) const {
            return typename Y_real_traits_1::Boundary_between()(r1, r2);
        }
    };
    CGAL_Algebraic_Kernel_cons(Boundary_between_y_2, 
            boundary_between_y_2_object);
    
    //! \brief comparison of x-coordinates 
    struct Compare_x_2 :
         public Binary_function<X_coordinate_1, X_coordinate_1, 
                Comparison_result > {

        Comparison_result operator()(const X_coordinate_1& x1, 
                                         const X_coordinate_1& x2) const {
        // not yet implemented in Algebraic_kernel_1 (will it be ?)
        //   Algebraic_kernel_1 ak;
        //   return (ak.compare_x_2_object()(x1, x2));
            return x1.compare(x2);
        }
        Comparison_result operator()(const Xy_coordinate_2& xy1, 
                                         const Xy_coordinate_2& xy2) const {
            return ((*this)(xy1.x(), xy2.x()));
        }
    };
    CGAL_Algebraic_Kernel_pred(Compare_x_2, compare_x_2_object);

    //! \brief comparison of y-coordinates of two points
    struct Compare_y_2 :
        public Binary_function< Xy_coordinate_2, Xy_coordinate_2, 
                Comparison_result > {
        
        Comparison_result operator()(const Xy_coordinate_2& xy1, 
                                     const Xy_coordinate_2& xy2) const {
            
            // It is easier if the x coordinates are equal!
            if(Compare_x_2()(xy1.x(),xy2.x()) == CGAL::EQUAL) {
                return Compare_xy_2()(xy1,xy2);
            }

            return (Compare_x_2()(xy1.y(), xy2.y()));
        }
    };
    CGAL_Algebraic_Kernel_pred(Compare_y_2, compare_y_2_object);
    
    //! lexicographical comparison of two objects of type \c Xy_coordinate_2
    //!
    //! \c equal_x specifies that only y-coordinates need to be compared
    struct Compare_xy_2 :
          public Binary_function<Xy_coordinate_2, Xy_coordinate_2, 
                Comparison_result > {

         Compare_xy_2(Algebraic_curve_kernel_2 *kernel) :
             _m_kernel(kernel) {
         }
    
         Comparison_result operator()(const Xy_coordinate_2& xy1, 
             const Xy_coordinate_2& xy2, bool equal_x = false) const {

             // handle easy cases first
             if(xy1.is_identical(xy2))
                return CGAL::EQUAL;
                
             if(equal_x && xy1.curve().is_identical(xy2.curve()))
                return CGAL::sign(xy1.arcno() - xy2.arcno());
                
             bool swap = (xy1.id() > xy2.id());
             std::pair<Xy_coordinate_2, Xy_coordinate_2> p(xy1, xy2);
             if(swap) {
                 p.first = xy2;
                 p.second = xy1;
             }
           
             typename Cmp_xy_map::Find_result r =
                _m_kernel->_m_cmp_xy.find(p);
             if(r.second) {
               //std::cerr << "Xy_coordinate2: precached compare_xy result\n";
                 return (swap ? -(r.first->second) : r.first->second);
             }

             CGAL::Comparison_result res =
                   p.first.compare_xy(p.second, equal_x);
             _m_kernel->_m_cmp_xy.insert(std::make_pair(p, res));
             return (swap ? -res : res);
        }

    private:
        Algebraic_curve_kernel_2 *_m_kernel;    
        
    };
    //CGAL_Algebraic_Kernel_pred(Compare_xy_2, compare_xy_2_object);
    Compare_xy_2 compare_xy_2_object() const {
        return Compare_xy_2((Algebraic_curve_kernel_2 *)this);
    }

    //! \brief checks whether curve has only finitely many self-intersection
    //! points, i.e., it has no self-overlapped continuous parts
    //!
    //! for algerbaic curves this means that supporting polynomial is 
    //! square-free
    struct Has_finite_number_of_self_intersections_2 :
            public Unary_function< Polynomial_2_CGAL, bool > {

        bool operator()(const Polynomial_2_CGAL& p) const {

            //typename Polynomial_traits_2::Is_square_free is_square_free;
            CGAL_error_msg("is_square_free is not yet supported\n");
            return true; //is_square_free(p);
        }

        bool operator()(const Internal_polynomial_2& p) const {
            NiX2CGAL_converter cvt;
            return (*this)(cvt(p));
        }
    };
    CGAL_Algebraic_Kernel_pred(Has_finite_number_of_self_intersections_2, 
            has_finite_number_of_self_intersections_2_object);
            
    //! \brief checks whether a curve pair has finitely many intersections,
    //! in other words, whether two curves have no continuous common part
    //!
    //! in case of algerbaic curves: checks whether supporting polynomials are
    //! coprime
    struct Has_finite_number_of_intersections_2 :
        public Binary_function< Polynomial_2_CGAL, Polynomial_2_CGAL, bool > {
         
        bool operator()(const Internal_polynomial_2& f,
                        const Internal_polynomial_2& g) const {
            // if curve ids are the same - non-decomposable
            if(f.id() == g.id())
                return true;
            NiX2CGAL_converter cvt;
            return (*this)(cvt(f), cvt(g));
        }
     
        bool operator()(const Polynomial_2_CGAL& f,
                        const Polynomial_2_CGAL& g) const {
            // if curve ids are the same - non-decomposable
            if(f.id() == g.id())
                return true;
            typename Polynomial_traits_2::Gcd_up_to_constant_factor gcd_utcf;
            typename Polynomial_traits_2::Total_degree total_degree;
            //NiX2CGAL_converter cvt;
            //Polynomial_2_CGAL p1 = cvt(c1.f()), p2 = cvt(c2.f());
            return (total_degree(gcd_utcf(f, g)) == 0);
        }
    };
    CGAL_Algebraic_Kernel_pred(Has_finite_number_of_intersections_2, 
            has_finite_number_of_intersections_2_object);
    
    //! set of various curve and curve pair decomposition functions
    struct Decompose_2 {
    
        //! default constructor
        Decompose_2(/*Self *pkernel_2*/)  
        {  }

        //! \brief returns a curve without self-overlapping parts 
        //!
        //! in case of algebraic curves computes square-free part of supporting
        //! polynomial
        Polynomial_2_CGAL operator()(const Polynomial_2_CGAL& p) {
            typename Polynomial_traits_2::Make_square_free msf;
            return msf(p);
        }
        
        //! temporary version for \c NiX::Polynomial
        Internal_polynomial_2 operator()(const Internal_polynomial_2& p) {
            NiX2CGAL_converter cvt;
            CGAL2NiX_converter cvt_back;
            return cvt_back((*this)(cvt(p)));
        }
        
        //! \brief computes a square-free factorization of a curve \c c, 
        //! returns the number of pairwise coprime square-free factors
        //! 
        //! returns square-free pairwise coprime factors in \c fit and
        //! multiplicities in \c mit. Template argument type of \c fit is
        //! \c Curve_analysis_2, and \c mit is \c int
        template< class OutputIterator1, class OutputIterator2 >
        int operator()(const Curve_analysis_2& ca,
                     OutputIterator1 fit, OutputIterator2 mit ) const {
                        
            typename Polynomial_traits_2::
                Square_free_factorization_up_to_constant_factor factorize;
            NiX2CGAL_converter cvt;
            std::vector<Polynomial_2_CGAL> factors;
            
            int n_factors = factorize(cvt(ca.polynomial_2()),
                 std::back_inserter(factors), mit);
            Construct_curve_2 cc_2;
            for(int i = 0; i < (int)factors.size(); i++) 
                *fit++ = cc_2(factors[i]);
            
            return n_factors;
        }
        
        /*!\brief
         * computes for a given pair of curves \c ca1 and \c ca2 their
         * common part \c oib and coprime parts \c oi1 and \c oi2
         * respectively; returns \c true if the curves were decomposed
         *
         * returns true if \c ca1 and \c ca2 are coprime. Template argument
         * type of \c oi{1,2,b} is \c Curve_analysis_2
         */
        template < class OutputIterator > 
        bool operator()(const Curve_analysis_2& ca1,
            const Curve_analysis_2& ca2, OutputIterator oi1,
                OutputIterator oi2, OutputIterator oib) const {

            Construct_curve_2 cc_2;
            typedef std::vector<Internal_curve_2> Curves;
            Curves parts_f, parts_g;
            
            if(Internal_curve_2::decompose(ca1._internal_curve(),
                    ca2._internal_curve(), std::back_inserter(parts_f),
                     std::back_inserter(parts_g))) {

                typename Curves::const_iterator cit;
                // this is temporary solution while curves are cached on
                // AlciX level
                CGAL_precondition(parts_f[0].f() == parts_g[0].f());
                *oib++ = cc_2(parts_f[0].f());
                
                if(parts_f.size() > 1)
                    for(cit = parts_f.begin() + 1; cit != parts_f.end(); cit++)
                        *oi1++ = cc_2(cit->f());
                if(parts_g.size() > 1)
                    for(cit = parts_g.begin() + 1; cit != parts_g.end(); cit++)
                        *oi2++ = cc_2(cit->f());
                return true;
            }
            // copy original curves to the output iterator:
            *oi1++ = ca1;
            *oi2++ = ca2;
            return false;
        }
    };
    CGAL_Algebraic_Kernel_cons(Decompose_2, decompose_2_object);
    
    //!@}
public:
    //! \name types and functors for \c CurvedKernelViaAnalysis_2
    //!@{
    
    typedef X_coordinate_1 Algebraic_real_1;
    typedef Xy_coordinate_2 Algebraic_real_2;
    
    typedef Has_finite_number_of_self_intersections_2 Is_square_free_2;
    typedef Has_finite_number_of_intersections_2 Is_coprime_2;

    typedef Decompose_2 Make_square_free_2;
    typedef Decompose_2 Square_free_factorization;
    typedef Decompose_2 Make_coprime_2;
    
    //! \brief computes the derivative w.r.t. the first (innermost) variable
    struct Derivative_x_2 : 
        public Unary_function< Polynomial_2_CGAL, Polynomial_2_CGAL > {
        
        Polynomial_2_CGAL operator()(const Polynomial_2_CGAL& p) const
        {
            typename Polynomial_traits_2::Derivative derivate;
            return derivate(p, 0);
        }
    };
    CGAL_Algebraic_Kernel_cons(Derivative_x_2, derivative_x_2_object);

    //! \brief computes the derivative w.r.t. the first (outermost) variable
    struct Derivative_y_2 :
        public Unary_function< Polynomial_2_CGAL, Polynomial_2_CGAL > {
        
        Polynomial_2_CGAL operator()(const Polynomial_2_CGAL& p) const
        {
            typename Polynomial_traits_2::Derivative derivate;
            return derivate(p, 1);
        }
    };
    CGAL_Algebraic_Kernel_cons(Derivative_y_2, derivative_y_2_object);

    struct X_critical_points_2 : 
        public Binary_function< Curve_analysis_2, 
            std::iterator<std::output_iterator_tag, Xy_coordinate_2>,
            std::iterator<std::output_iterator_tag, Xy_coordinate_2> > {
       
        //! \brief copies in the output iterator the x-critical points of
        //! polynomial \c p as objects of type \c Xy_coordinate_2
        //!
        //! all points (x, y) with f(x,y) = fy(x,y) = 0 are x-critical points
        //! (i.e, singularities are also counted)
        template <class OutputIterator>
        OutputIterator operator()(const Curve_analysis_2& ca_2,
                OutputIterator oi) const {
                
            typename Self::Derivative_x_2 der_x;
            Construct_curve_2 cc_2;
            Construct_curve_pair_2 ccp_2;
            NiX2CGAL_converter cvt;
            // construct curve analysis of a derivative in y
            Curve_analysis_2 ca_2x = cc_2(der_x(cvt(ca_2.polynomial_2())));
            Curve_pair_analysis_2 cpa_2 = ccp_2(ca_2, ca_2x);
            typename Curve_pair_analysis_2::Status_line_1 cpv_line;
            typename Curve_analysis_2::Status_line_1 cv_line;
            
            int i, j, n_arcs, n_events =
                cpa_2.number_of_status_lines_with_event();
            std::pair<int,int> ipair;
            bool vline_constructed = false;
            
            for(i = 0; i < n_events; i++) {
                cpv_line = cpa_2.status_line_at_event(i);
                // no 2-curve intersections over this status line
                if(!cpv_line.is_intersection())
                    continue;
                n_arcs = cpv_line.number_of_events();
                for(j = 0; j < n_arcs; j++) {
                    ipair = cpv_line.curves_at_event(j);
                    if(ipair.first == -1||ipair.second == -1) 
                        continue;
                    if(!vline_constructed) {
                        cv_line = ca_2.status_line_at_exact_x(cpv_line.x());
                        vline_constructed = true;
                    }
                    // ipair.first is an arcno over status line of the
                    // curve p
                    *oi++ = cv_line.algebraic_real_2(ipair.first);
                }
                vline_constructed = false;
            }
            return oi;
        }
        
        //! \brief computes the ith x-critical point of polynomial \c p
        Xy_coordinate_2 operator()(const Curve_analysis_2& ca, int i) const
        {
            std::vector<Xy_coordinate_2> x_points;
            (*this)(ca, std::back_inserter(x_points));
            CGAL_precondition(0 >= i&&i < x_points.size());
            return x_points[i];
        }
    };
    CGAL_Algebraic_Kernel_cons(X_critical_points_2,
        x_critical_points_2_object);
    
    struct Y_critical_points_2 :
        public Binary_function< Curve_analysis_2, 
            std::iterator<std::output_iterator_tag, Xy_coordinate_2>,
            std::iterator<std::output_iterator_tag, Xy_coordinate_2> > {
    
        //! \brief copies in the output iterator the y-critical points of
        //! polynomial \c p as objects of type \c Xy_coordinate_2
        template <class OutputIterator>
        OutputIterator operator()(const Curve_analysis_2& ca_2, 
            OutputIterator oi) const
        {
            Construct_curve_2 cc_2;
            Construct_curve_pair_2 ccp_2;
            
            typename Curve_analysis_2::Status_line_1 cv_line;
            std::pair<int,int> ipair;
            int i, j, k, n_arcs, n_events =
                ca_2.number_of_status_lines_with_event();
            
            bool cpa_constructed = false, vline_constructed = false; 
            typename Curve_pair_analysis_2::Status_line_1
                cpv_line;
            Curve_pair_analysis_2 cpa_2;
            
            for(i = 0; i < n_events; i++) {
                cv_line = ca_2.status_line_at_event(i);
                n_arcs = cv_line.number_of_events();
                for(j = 0; j < n_arcs; j++) {
                    ipair = cv_line.number_of_incident_branches(j);
                    // general case: no special tests required
                    if(!(ipair.first == 1&&ipair.second == 1)) {
                        *oi++ = cv_line.algebraic_real_2(j);
                        continue;
                    }
                    if(!cpa_constructed) {
                        typename Self::Derivative_y_2 der_y;
                        NiX2CGAL_converter cvt;
                        // construct curve analysis of a derivative in x
                        Curve_analysis_2 ca_2y =
                            cc_2(der_y(cvt(ca_2.polynomial_2())));
                        cpa_2 = ccp_2(ca_2, ca_2y);
                        cpa_constructed = true;
                    }
                    if(!vline_constructed) {
                        cpv_line = cpa_2.status_line_for_x(cv_line.x());
                        vline_constructed = true;
                    }
                    if(!cpv_line.is_intersection())
                        continue;
                    // obtain the y-position of j-th event of curve p
                    k = cpv_line.event_of_curve(j, 0);
                    ipair = cpv_line.curves_at_event(k);
                    
                    // pick up only event comprised of both curve and its der
                    if(ipair.first != -1&&ipair.second != -1)
                        *oi++ = cv_line.algebraic_real_2(j);
                }
                vline_constructed = false;
            }
            return oi;
        }
        
        //! \brief computes the ith y-critical point of polynomial \c p
        Xy_coordinate_2 operator()(const Curve_analysis_2& ca, int i) const
        {
            std::vector<Xy_coordinate_2> y_points;
            (*this)(ca, std::back_inserter(y_points));
            CGAL_precondition(0 >= i&&i < y_points.size());
            return y_points[i];
        }
    };
    CGAL_Algebraic_Kernel_cons(Y_critical_points_2,
        y_critical_points_2_object);

    /*!\brief 
     * computes the sign of a bivariate polynomial \c p evaluated at the root 
     * \c r of a system of two bivariate polynomial equations
     *
     * returns a value convertible to \c CGAL::Sign
     */
    struct Sign_at_2 :
        public Binary_function< Curve_analysis_2, Xy_coordinate_2, Sign > {

        typedef typename Y_real_traits_1::Boundary Boundary;
        typedef boost::numeric::interval<Boundary> Interval;
        
        typedef CGAL::Polynomial<Boundary> Poly_rat_1;
        typedef CGAL::Polynomial<Poly_rat_1> Poly_rat_2;
        
        Sign operator()(const Curve_analysis_2& ca_2,
                const Xy_coordinate_2& r) const {
                
            if(ca_2.is_identical(r.curve()) || _test_exact_zero(ca_2, r))
                return CGAL::ZERO;
            
            Interval ix = r.get_approximation_x();
            Interval iy = r.get_approximation_y();

            Boundary x_len = ix.upper() - ix.lower(),
                y_len = iy.upper() - iy.lower();

            while(1) {
                Interval iv = r.interval_evaluate_2(ca_2.polynomial_2());
                CGAL::Sign s_lower = CGAL::sign(iv.lower());
                if(s_lower == sign(iv.upper()))
                    return s_lower;
                
                if(x_len > y_len) {
                    r.refine_x();
                    ix = r.get_approximation_x();
                    x_len = ix.upper() - ix.lower();
                } else {
                    r.refine_y();
                    iy = r.get_approximation_y();
                    y_len = iy.upper() - iy.lower();
                }
            }
        }
        
    protected:

        bool _test_exact_zero(const Curve_analysis_2& ca_2,
            const Xy_coordinate_2& r) const {

            Internal_polynomial_2 zero_p(Coefficient(0));
            if (ca_2.polynomial_2() == zero_p) {
                return true;
            }

            Construct_curve_2 cc_2;
            Construct_curve_pair_2 ccp_2;
            typename Curve_analysis_2::Status_line_1
                cv_line = ca_2.status_line_for_x(r.x());
            // fast check for the presence of status line at r.x()
            if(cv_line.covers_line())    
                return true;

            // Handle non-coprime polynomial
            Internal_polynomial_2 gcd = Self::gcd_cache_2()
               (std::make_pair(ca_2.polynomial_2(), r.curve().polynomial_2()));

            Curve_analysis_2 gcd_curve = cc_2(gcd);
            if(NiX::total_degree(gcd)>0) {
                
                Construct_curve_pair_2 ccp_2;
                Curve_analysis_2 r_curve_remainder =
                    cc_2(NiX::div_utcf(r.curve().polynomial_2(), gcd));
                    
                r.simplify_by(ccp_2(gcd_curve, r_curve_remainder));
                if(r.curve().polynomial_2() == gcd) 
                    return true;
            }

            Curve_pair_analysis_2 cpa_2 = ccp_2(ca_2, r.curve());
            typename Curve_pair_analysis_2::Status_line_1
                cpv_line = cpa_2.status_line_for_x(r.x());
            
            if(cpv_line.is_event() && cpv_line.is_intersection()) {
                // get an y-position of the point r
                int idx = cpv_line.event_of_curve(r.arcno(), 1);
                std::pair<int, int> ipair =
                      cpv_line.curves_at_event(idx);
                if(ipair.first != -1&&ipair.second != -1)
                    return true;
            }
            return false;
        }
    
    };
#if 0
    struct Sign_at_2_buggy :
        public Binary_function< Polynomial_2, Xy_coordinate_2, Sign > {
        
        Sign operator()(const Polynomial_2& p, const Xy_coordinate_2& r) const
        {
            if(p.id() == r.curve().id()) // point lies on the same curve
                return CGAL::ZERO;
           
            Curve_analysis_2 ca_2(p), ca_2r(r.curve());
            Curve_pair_analysis_2 cpa_2(ca_2, ca_2r);
            typename Curve_analysis_2::Status_line_1
                cv_line = ca_2.status_line_for_x(r.x()),
                cv_line_r = ca_2r.status_line_for_x(r.x());
            
            // fast check for the presence of vertical line at r.x()
            if(cv_line.covers_line())    
                return CGAL::ZERO;
                
            // in case there is no event at this x-coordinate, status
            // line at some rational x over an interval is returned
            typename Curve_pair_analysis_2::Status_line_1
                cpv_line = cpa_2.status_line_for_x(r.x());
                        
            // get an y-position of the point r
            int idx = cpv_line.event_of_curve(r.arcno(), 1);
            std::pair<int, int> ipair;
            X_coordinate_1 boundary_x;
            
            if(cpv_line.is_event()) {
                if(cpv_line.is_intersection()) {
                    ipair = cpv_line.curves_at_event(idx);
                    // easy case: there is a 2-curve intersection at this x
                    if(ipair.first != -1&&ipair.second != -1)
                        return CGAL::ZERO; // intersection of both curves
                }
                // check if there is an event of curve p at r.x()
                if(cv_line.is_event()) {
                    if(cv_line_r.is_event())
                        CGAL_error_msg("you're lucky )) this is not an easy \
                            case..");
                    //std::cout << "sign at event of curve p\n";
                    // this is an event of curve p but not of r.curve() ->
                    // shift to the left of r.x() otherwise we would find
                    // arc-numbers of p at event point (r.arcno() is valid
                    // over curve-pair interval)
                    cpv_line = cpa_2.status_line_of_interval(
                        cpv_line.index());
                    // recompute vertical line of p and y-position of r
                    // (however y-position of r.arcno() should not change
                    // since it can only happen at 2-curve event or at event
                    // of g)
                    idx = cpv_line.event_of_curve(r.arcno(), 1);
                    //  need cv-line over interval ?
                    cv_line = ca_2.status_line_of_interval(
                        cv_line.index());
                    boundary_x = cpv_line.x();
                    
                } else if(cv_line_r.is_event()) {
                
                   // std::cout << "sign at event of curve r: cpv_line x: "
                     //   << cpv_line.x() << "\n";
                // this is an event of r.curvve() -> therefore cpv_line.x()
                // is given as algebraic real (not rational) and r.arcno() is
                // an event arcno 
                    // need to recompute boundary_x (but leave cpv_line
                    // unchanged otherwise r.arcno() is not valid)
                    boundary_x = cpa_2.status_line_of_interval(
                        cpv_line.index()).x();
                }
            } else {
            // there is no event at r.x() of curve p hence we're free to
            // pick up any rational boundary over an interval to compute the
            // sign at 
                boundary_x = cpv_line.x();
               // std::cout << "sign over curve pair interval\n";
            }
            // obtain status line at exact rational x
            cv_line = ca_2.status_line_at_exact_x(
                X_coordinate_1(boundary_x.low()));
            
            int arcno_low = -1, arcno_high = -1, i = idx;
            typedef typename Self::Algebraic_real_traits Traits;
            typedef typename Traits::Boundary Boundary;
            Boundary boundary_y;
            Xy_coordinate_2 xy1, xy2;
            
            // arcno_low and arcno_high are consecutive event indices of
            // curve p at r.x()
            while(i-- > 0) {
                ipair = cpv_line.curves_at_event(i);
                if(ipair.first != -1) {
                    arcno_low = ipair.first;
                    xy1 = cv_line.algebraic_real_2(arcno_low);
                    break;
                }
            }
            i = idx;
            while(i++ < cpv_line.number_of_events() - 1) {
                ipair = cpv_line.curves_at_event(i);
                if(ipair.first != -1) {
                    arcno_high = ipair.first;
                    xy2 = cv_line.algebraic_real_2(arcno_high);
                    break;
                }
            }
                   
            if(arcno_low != -1) {
                boundary_y = (arcno_high != -1 ? 
                     typename Traits::Boundary_between()(xy1, xy2) :
                     typename Traits::Upper_boundary()(xy1));
            } else {
                // if arcno_high == -1 pick up arbitrary rational since the
                // curve p does not cross vertical line at r.x()
                boundary_y = (arcno_high != -1 ? 
                     typename Traits::Lower_boundary()(xy2) : Boundary(0)); 
            }
            if(boundary_x.low() != boundary_x.high())
                std::cout << "oops very bizarre error occurred..\n";
              
            NiX::Polynomial<Boundary> poly = 
                NiX::substitute_x(p.f(), boundary_x.low());
                            
            return poly.sign_at(boundary_y);
        }
    };
#endif
    CGAL_Algebraic_Kernel_pred(Sign_at_2, sign_at_2_object);

    /*!\brief
     * copies in the output iterator \c roots the common roots of polynomials
     * \c p1 and \c p2 and copies in the output iterator \c mults their 
     * respective multiplicity as intergers, in the same order
     *
     * template argument type of \c roots is \c Xy_coordinate_2 , returns the
     * pair of respective past-the-end iterators
     *
     * \pre p1 and p2 are square-free and the set of solutions of the system
     * is 0-dimensional
     */  
    struct Solve_2 {
    
        typedef Curve_analysis_2 first_argument_type;
        typedef Curve_analysis_2 second_argument_type;
        typedef std::iterator<std::output_iterator_tag, Xy_coordinate_2>
            third_argument_type;
        typedef std::iterator<std::output_iterator_tag, int>
            fourth_argument_type;
        typedef std::pair<third_argument_type, fourth_argument_type>
            result_type;
     
        template <class OutputIteratorRoots, class OutputIteratorMult>
        std::pair<OutputIteratorRoots, OutputIteratorMult>
           operator()(const Curve_analysis_2& ca1, const Curve_analysis_2& ca2,
                OutputIteratorRoots roots, OutputIteratorMult mults) const
        {
            // these tests are quite expensive... do we really need them ??
            /*
            CGAL_precondition_code (
                typename Self::Has_finite_number_of_self_intersections_2 
                    not_self_overlapped;
                typename Self::Has_finite_number_of_intersections_2 
                    do_not_overlap;
                CGAL_precondition(not_self_overlapped(ca1) &&
                    not_self_overlapped(ca2));
                CGAL_precondition(do_not_overlap(ca1, ca2));
            );
            */
            Construct_curve_pair_2 ccp_2;
            Curve_pair_analysis_2 cpa_2 = ccp_2(ca1, ca2);
            typename Curve_pair_analysis_2::Status_line_1 cpv_line;
            // do we need to check which supporting curve is simpler ?    
            typename Polynomial_traits_2::Total_degree total_degree;
            NiX2CGAL_converter cvt;
            Polynomial_2_CGAL f1 = cvt(ca1.polynomial_2()),
                f2 = cvt(ca2.polynomial_2());
            bool first_curve = (total_degree(f1) < total_degree(f2));
            
            int i, j, n = cpa_2.number_of_status_lines_with_event();
            std::pair<int, int> ipair;
            for(i = 0; i < n; i++) {
                cpv_line = cpa_2.status_line_at_event(i);
                if(!cpv_line.is_intersection())
                    continue;
                // store x-coord for future use
                X_coordinate_1 x = cpv_line.x(); 
                for(j = 0; j < cpv_line.number_of_events(); j++) {
                    ipair = cpv_line.curves_at_event(j);
                    if(ipair.first == -1 || ipair.second == -1) 
                        continue;
                    // VOILA!! we've got it !!!
                    *roots++ = Xy_coordinate_2(x, (first_curve ? ca1 : ca2),
                            (first_curve ? ipair.first: ipair.second));
                    *mults++ = cpv_line.multiplicity_of_intersection(j);
                }
            }
            return std::make_pair(roots, mults);
        }
    };
    CGAL_Algebraic_Kernel_cons(Solve_2, solve_2_object);

#undef CGAL_Algebraic_Kernel_pred    
#undef CGAL_Algebraic_Kernel_cons 
    
    //!@}
    
}; // class Algebraic_curve_kernel_2

CGAL_END_NAMESPACE

#endif // CGAL_ALGEBRAIC_CURVE_KERNEL_2_H
