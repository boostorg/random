#ifndef BOOST_RANDOM_DYNAMIC_DISCRETE_DISTRIBUTION_HPP_INCLUDED
#define BOOST_RANDOM_DYNAMIC_DISCRETE_DISTRIBUTION_HPP_INCLUDED
#include <vector>
#include <limits>
#include <istream>
#include <ostream>
#include <queue>
#include <cmath>
#include <string>
#include <algorithm>
#include <initializer_list>
#include <boost/assert.hpp>
#include <boost/align/aligned_allocator.hpp>
#include <boost/random/detail/config.hpp>
#include <boost/random/detail/operators.hpp>
#include <boost/random/detail/vector_io.hpp>
#include <boost/random/generate_canonical.hpp>



namespace boost { namespace random{

/////////////////////////
// Underlying tree data structure
namespace detail{
template <class LiteralIntType, class Real = double, int Fanout = 16>
    class complete_kary_complete_tree {
        static constexpr typename std::make_unsigned<LiteralIntType>::type unsigned_fanout = Fanout;


        public:

        // Compute minimal complete k-ary tree size to store exactly n leaves
        // without needing bounds checks during selection. returns the index of the first leaf and the total number of leaves
        // Returns {total_nodes_excluding_root, leaf_start_index}
        std::pair<LiteralIntType, LiteralIntType> minimal_tree_shape(LiteralIntType n) {

            if (n == 0) return {0, 0}; // no internal nodes, no leaves

            LiteralIntType k = boost::core::countr_zero(unsigned_fanout);
            typename std::make_unsigned<LiteralIntType>::type unsigned_n = static_cast<typename std::make_unsigned<LiteralIntType>::type>(n);
            LiteralIntType max_leaf = LiteralIntType(1) << (((boost::core::bit_width(unsigned_n - 1) + k - 1) / k) * k);

            LiteralIntType leaf_start = 0; //the index of the first node that can contain leaves
            LiteralIntType level = 0; //the index of the first node in the bottom row

            while (true) {
                level = first_child_of(level);
                if (level >= max_leaf) break;
                leaf_start=level;
            }

            LiteralIntType total_nodes = leaf_start+n;
            return {total_nodes, leaf_start};
        }



        static_assert((Fanout & (Fanout - 1)) == 0, "fanout must be power of two");

        using position_type = LiteralIntType;

        //constructors
        complete_kary_complete_tree() : data_(1, Real(0)), leaf_start_(0), leaf_count_(0) {}

        explicit complete_kary_complete_tree(LiteralIntType leaf_count) {
            resize(leaf_count);
        }

        // Resize to accommodate exactly n leaves
            void resize(LiteralIntType n) {
                if (n == 0) {
                    data_.assign(1, Real(0));
                    leaf_start_ = 0;
                    leaf_count_ = 0;
                    return;
                }

                 std::pair<LiteralIntType, LiteralIntType> treeshape = minimal_tree_shape(n);
                 LiteralIntType total_nodes = treeshape.first;
                // allocate all internal nodes + leaves (root is kept separately by the derived class)
                data_.resize(total_nodes);
                leaf_count_ = n;
                leaf_start_ = treeshape.second;
            }



            LiteralIntType size() const {
                return data_.size();
            }

            Real& value_of(position_type p) {
                BOOST_ASSERT(p < data_.size());

                return data_[p];
            }

            const Real& value_of(position_type p) const {
                BOOST_ASSERT(p < data_.size());
                return data_[p];
            }

            // Tree navigation (-1 based indexing). root stored separately
        

            position_type parent_of(position_type i) const {
                BOOST_ASSERT(i > 0);
                return (i >> log2_fanout) - 1;
            }

            position_type first_child_of(position_type i) const {
                return (i + 1) << log2_fanout;
            }

            position_type child_index(position_type parent, LiteralIntType child_num) const {
                return first_child_of(parent) + child_num;
            }

            bool is_leaf(position_type i) const {
                return i >= leaf_start_;
            }

            LiteralIntType leaf_start() const {
                return leaf_start_;
            }

            LiteralIntType leaf_count() const {
                return leaf_count_;
            }

            std::vector<Real,boost::alignment::aligned_allocator<Real, 128> > &data() {return data_;}

            const std::vector<Real,boost::alignment::aligned_allocator<Real, 128> > &data() const {return data_;}

        private:
            std::vector<Real,boost::alignment::aligned_allocator<Real, 128> > data_;
            LiteralIntType leaf_start_;
            LiteralIntType leaf_count_;
            LiteralIntType max_leaf_;
            static constexpr LiteralIntType fanout = Fanout; 



            static constexpr LiteralIntType log2_fanout = core::countr_zero(unsigned_fanout);
        };}

///////////////////////


/**
 * @brief dynamic_discrete_distribution is a highly optimized library for weighted random selection modeled after std::discrete_distribution. It selects an index where the index's probability of being selected is weighted by inputted weights. It satisfies all RandomNumberDistribution requirements except for the requirement of constant time equality comparisons between both distribution objects and parameter objects, which is not possible in discrete distributions.  
 * * This library is designed to be used in discrete event simulation applications in which there is a set of events with different probabilities of occurring, and the simulation must choose an event to occur and update probabilities of other events accordingly. These simulations often involve many more updates than selections, but exact ratios of update to selection differ depending on the application. This library provides efficient update and selection in which the underlying tree data structure can be tuned at compile time to prioritize update over selection to varying degrees. 
 * @tparam IntType is the type of the integers returned by operator(), which is the selection function. IntType must be an unsigned integer type
 * @tparam Real is the type of the weights
 * @tparam Fanout, which must be an positive integer power of 2, controls the branching factor of the underlying complete tree data structure and can be adjusted to change the amount that update is prioritized over selection. update_weight has a runtime of $O(log_{\text{fanout}} N)$ while selection (operator ()) has a runtime of $O( \text{fanout} * (log_{\text{fanout}} N))$ . Additionally, trees with larger fanouts use less memory. 
 * @tparam Precision controls the number of bits of randomness generated during selection. 
 */
template <
    class IntType = int,
    class Real = double,
    int Fanout = 16,
    size_t Precision = std::numeric_limits<Real>::digits
>
class dynamic_discrete_distribution {
    //This protects against non literal IntType types (necessary because of the boost::multiprecision::cpp_int types). indexes can't be larger than size_t anyway, so we just use size_t. We use this in the place of IntType for internal work, and cast to IntType before returning
    using LiteralIntType = typename std::conditional<std::is_trivially_destructible<IntType>::value && std::is_constructible<IntType>::value,IntType,size_t>::type;

    static constexpr typename std::make_unsigned<LiteralIntType>::type unsigned_fanout = Fanout;
    static_assert(Fanout>0 && boost::core::has_single_bit(unsigned_fanout),"template parameter Fanout must be a positive power of 2");
    using This = dynamic_discrete_distribution<IntType, Real, Fanout, Precision>;


public:
    using input_type = Real;  
    using result_type = IntType; 

    /**
     * @brief standard library random number distributions separate state related to the parameters of a distribution and state related to the generation of random numbers by defining a member class param_type that holds the distribution parameters and related data structures. The distribution stores its parameter set in a param_type object. 
     */
    class Param : protected detail::complete_kary_complete_tree<LiteralIntType, Real, Fanout> {
        using base_tree = detail::complete_kary_complete_tree<LiteralIntType, Real, Fanout>;
        using pos_type = typename base_tree::position_type;

    public:
        using distribution_type = This;
        
        /**
         * @brief constructs a `param_type` parameter set with a single element 0 with a weight of 1. 
         */
        Param() : Param({1.0}) {}

        /**
         * @brief constructs a `param_type` parameter set with weights equal to the value in `weights` at each index.
         */
        Param(const std::vector<Real>& weights)
            : Param(weights.begin(), weights.end()) {}

        /**
         * @brief constructs a `param_type` parameter set with the weights in the initializer_list `il`. Each weight will have the same index as it did in il. 
         */
        Param(const std::initializer_list<Real>& il)
            : Param(il.begin(), il.end()) {}
  
        /**
         * @brief Constructs a `param_type` parameter set with `count` weights that are generated using function `unary_op`. Each of the weights is equal to ${w_i} = unary_op(xmin + δ(i + 0.5))$, where $δ = (xmax − xmin)count$ and $i ∈ {0, ..., count − 1}$. `xmin` and `xmax` must be such that `δ > 0`. If `count == 0` the effects are the same as of the default constructor.
         */
        template< class UnaryOperation >
        Param( std::size_t count, double xmin, double xmax, UnaryOperation unary_op )
            : Param(weight_distribution(count,xmin,xmax,unary_op)) {}

        /**
         * @brief constructs the distribution from the first and last iterators to a collection of weights of type `weight_type`. Each weight is indexed according to the number of elements between itself and first. 
         */
        template<class InputIt>
        Param(InputIt first, InputIt last)
            : base_tree()
        {
            size_t distance = std::distance(first, last);
            LiteralIntType n = distance;
            typename std::make_unsigned<LiteralIntType>::type n_unsigned = n;

            //n = 2;

            // Round up leaves to nearest full complete k-ary tree level (power of fanout)
            if (n <= fanout){
                max_leaf_ = fanout;
                num_layers_ = 1;
            }
            else{
                LiteralIntType k = boost::core::countr_zero(unsigned_fanout);
                num_layers_ = (boost::core::bit_width(n_unsigned - 1) + k - 1) / k;
                max_leaf_ = LiteralIntType(1) << (((boost::core::bit_width(n_unsigned - 1) + k - 1) / k) * k);
            }

            leaf_start_ = base_tree::minimal_tree_shape(n).second;
            base_tree::resize(n);
            leaf_end_ = n;


            // Copy weights to leaves, pad with zeros
            InputIt it = first;
            for (LiteralIntType i = leaf_start_; i < leaf_start_ + n; ++i) {
                weightsum_of(i) = std::max(Real(*it), Real(0));
                ++it;
            }

            // Build sums bottom-up from leaves to root (excluding root)
            for (ptrdiff_t i = leaf_start_ - 1; i >= 0; --i) {
                Real sum = 0;
                pos_type first_child = (i + 1) * fanout; // -1-index adjustment
                for (LiteralIntType c = 0; c < fanout; ++c) {
                    pos_type child = first_child + c;
                    if (child >= leaf_start_+n) break;
                    sum += weightsum_of(child);
                }
                weightsum_of(i) = sum;
            }

            // Compute root separately
            Real sum = 0;
            pos_type first_child = 0 * fanout; // root's first child in array
            if (leaf_start_ == 0){
                for (LiteralIntType c = 0; c < leaf_end_; ++c) {
                    pos_type child = first_child + c;
                    if (child >= max_leaf_) break;
                    sum += weightsum_of(child);
                }
            }
            else{
                for (LiteralIntType c = 0; c < fanout; ++c) {
                    pos_type child = first_child + c;
                    if (child >= max_leaf_) break;
                    sum += weightsum_of(child);
                }
            }
            total_weight_ = sum;
        }

        /**
         * @brief returns a vector of probabilities of each integer that could be generated by a dynamic_discrete_distribution using this parameter set. 
         */
        std::vector<Real> probabilities() const {
            std::vector<Real> probs(leaf_end_, Real(0));
            Real total = total_weight();
            if (total <= Real(0)) return probs;

            for (LiteralIntType i = 0; i < leaf_end_; ++i) {
                probs[i] = weightsum_of(leaf_start_ + i) / total;
            }
            return probs;
        }

        /**
         * @brief returns the minimum integer that could be generated by a dynamic_discrete_distribution using this parameter set assuming that it is not empty. This minimum will always be 0. 
         */
        static constexpr result_type min BOOST_PREVENT_MACRO_SUBSTITUTION () { return 0; }


        /**
         * @brief returns the maximum integer that could be generated by a dynamic_discrete_distribution using this parameter set assuming that it is not empty. 
         */
        result_type max BOOST_PREVENT_MACRO_SUBSTITUTION () const { return leaf_end_ == 0 ? 0 : leaf_end_ - 1; }

        /**
         * @brief updates weight of int`i`  to `new_weight`. If i is not in this parameter set, the behavior is undefined. 
         */
        void update_weight(IntType i, Real new_weight) {
            LiteralIntType i_literal = i;
            BOOST_ASSERT(new_weight >= Real(0));
            BOOST_ASSERT(i_literal <= leaf_end_);
            BOOST_ASSERT(i_literal>=0);
            i_literal = leaf_start_ + i_literal;
            Real diff = new_weight - weightsum_of(i_literal);
            weightsum_of(i_literal) = new_weight;
            total_weight_ += diff;


            while (i_literal >=fanout) {                
                i_literal = base_tree::parent_of(i_literal);

                weightsum_of(i_literal) += diff;
            }
        }

        /**
         * @brief gets the weight of int `i`. If i is not in the parameter set, the behavior is undefined. 
         */
        Real get_weight(IntType i) const {
            return get_weight_literal(i); 
        }

        /**
         * @brief returns the number of elements in this parameter set. 
         */
        IntType size() const { return leaf_end_; }

        /**
         * @brief returns the total of all of the weights in this parameter set.
         */
        Real total_weight() const noexcept {
            if (base_tree::size() == 0) return Real(0);
            return total_weight_;
        }

        /**
         * @brief adds weights.size() elements to the end of this parameter set with weights given by `weights` vector. 
         */
        void push_back(const std::vector<Real>& weights){
            for (Real w : weights) {
                push_back(w);
            }
        }

        /**
         * @brief adds an element to the end with weight `weight`.
         */
        void push_back(Real weight) {
            expand(leaf_end_+1);   
            update_weight(leaf_end_, weight);
            leaf_end_++;
        }

        /**
         * @brief removes the `count` highest integers. If the parameter set is empty, this function's behavior is undefined. NOTE: in this implementation, this does not deallocate memory or decrease the depth of the underlying tree based data structure. For the purposes of asymptotic analysis, consider "N" to be the largest number of elements ever held in this parameter set. 
         */
        void pop_back(IntType count) {
            for(IntType i = 0; i < count; ++i) {
                pop_back();
            }
        }

        /**
         * @brief removes the highest integer. If the parameter set is empty, this function's behavior is undefined. NOTE: in this implementation, this does not deallocate memory or decrease the depth of the underlying tree based data structure. For the purposes of asymptotic analysis, consider "N" to be the largest number of elements ever held in this parameter set. 
         */
        void pop_back() {
            BOOST_ASSERT(leaf_end_ > 0);
            leaf_end_--;
            update_weight(leaf_end_, Real(0));
        }

        /**
         * @brief compares the weights of each element in `rhs` and `lhs` for equality.
         */
        BOOST_RANDOM_DETAIL_EQUALITY_OPERATOR(Param, lhs, rhs){
            LiteralIntType sizer = rhs.literal_size();
            if (sizer != lhs.literal_size()){
                return false;
            }
            for (LiteralIntType i=0; i<sizer; i++){
                if (rhs.get_weight_literal(i)!=lhs.get_weight_literal(i)){
                    return false;
                }
            }
            return true;
        }

//         //PERIN TODO - DELETE METHOD
//         // Print tree for debugging (-1-indexed)
//   void printTree(std::ostream& os = std::cout) const {
//     IntType total_nodes = base_tree::size();

//     if (total_nodes == 0) {
//         os << "(empty tree)\n";
//         return;
//     }

//     std::queue<ptrdiff_t> q; // use signed for -1 root
//     q.push(-1); // -1 represents the root stored separately

//     IntType level = 0;

//     while (!q.empty()) {
//         IntType level_size = q.size();
//         os << "Level " << level << ": ";

//         for (IntType i = 0; i < level_size; ++i) {
//             ptrdiff_t node = q.front();
//             q.pop();

//             Real w = (node == -1) ? total_weight_ : weightsum_of(static_cast<pos_type>(node));
//             os << "[" << node << "]=" << w << "  ";

//             // enqueue children
//             pos_type first_child = (node + 1) * fanout; // -1-index adjustment
//             for (IntType c = 0; c < fanout; ++c) {
//                 pos_type child = first_child + c;
//                 if (child >= total_nodes) break;
//                 q.push(child);
//             }
//         }

//         os << "\n";
//         ++level;
//     }
//   }

        /**
         * @brief compares the weights of each element in `rhs` and `lhs` for inequality
         */
         BOOST_RANDOM_DETAIL_INEQUALITY_OPERATOR(Param)

        /**
         * @brief Restores the parameters with data read from `stream`. The formatting flags of `stream` are unchanged. The data must have been written using a stream with the same locale, `CharT` and `Traits` template parameters, otherwise the behavior is undefined. If bad input is encountered, stream.setstate(std::ios::failbit) is called, which may throw std::ios_base::failure. the parameter set is unchanged in that case.
         */
         BOOST_RANDOM_DETAIL_ISTREAM_OPERATOR(stream, Param, dist){
           std::vector<Real> newWeights;
           detail::read_vector(stream, newWeights);
           if (stream){
            dist = Param(newWeights);
           }


            return stream;
        }

        /**
         * @brief Writes a textual representation of the parameters to `stream`. The formatting flags and fill character of `stream` are unchanged.
         */
        BOOST_RANDOM_DETAIL_OSTREAM_OPERATOR(stream, Param, dist){
            std::vector<Real> leaves(dist.base_tree::data().begin()+dist.leaf_start_, dist.base_tree::data().begin()+dist.leaf_start_+dist.leaf_end_ );
            detail::print_vector(stream, leaves);

            return stream;
        }

    private:
        //size for internal use, involving fewer casts.
        LiteralIntType literal_size()const{
            return leaf_end_;
        }

        //get_weight for internal use
        Real get_weight_literal(LiteralIntType i) const {
            BOOST_ASSERT(i <= leaf_end_+leaf_start_);
            BOOST_ASSERT(i>=0);
            return weightsum_of(leaf_start_ + i);
        }

        Real& weightsum_of(pos_type p) { return base_tree::value_of(p); }
        const Real& weightsum_of(pos_type p) const { return base_tree::value_of(p); }
        
        // Expand from current leaf_count_ to new_leaf_count (must be larger), structurally, without recomputing
        // new expand: argument is NEW_LEAF_COUNT (number of leaves you want after expansion)
        void expand(LiteralIntType new_leaf_count) {
            if (new_leaf_count <= leaf_end_) return; // nothing to do
            auto &data_ = base_tree::data();
            if (new_leaf_count > max_leaf_) {
                max_leaf_ *= fanout; 
                // old shape
                std::pair<LiteralIntType, LiteralIntType> oldShape = base_tree::minimal_tree_shape(leaf_end_); 
                LiteralIntType old_total_nodes = oldShape.first;

                // new shape
                 std::pair<LiteralIntType, LiteralIntType> newShape = base_tree::minimal_tree_shape(new_leaf_count);
                 LiteralIntType new_total_nodes = newShape.first;
                 leaf_start_ = newShape.second;

                std::vector<LiteralIntType> old_starts;
                std::vector<LiteralIntType> old_levels;

                std::vector<LiteralIntType> new_starts;
                std::vector<LiteralIntType> new_levels;

                LiteralIntType position = 0;
                LiteralIntType sum = 0;

                // Creating the level lengths and level starts for the old size
                while (true) {
                    position = base_tree::first_child_of(position);
                    old_starts.push_back(sum);
                    LiteralIntType level = position - sum;
                    sum += level;
                    old_levels.push_back(level);
                    if (position >= old_total_nodes) break;
                }

                // Creating the level lengths and level starts for the new size
                position = 0;
                sum = 0;
                while (true) {
                    position = base_tree::first_child_of(position);
                    new_starts.push_back(sum);
                    LiteralIntType level = position - sum;
                    sum += level;
                    new_levels.push_back(level);
                    if (position >= new_total_nodes) break;
                }

                // allocate new array (zero-initialized)
                std::vector<Real,boost::alignment::aligned_allocator<Real,128>> new_data(new_total_nodes, Real(0));

                // copy each old level block into the next-deeper level of the new layout.
                // old level i -> new level (i+1). That packs the old block contiguously at the start
                // of the larger new level (the remainder stays zero).
                for (LiteralIntType i = 0; i < old_levels.size(); ++i) {
                    LiteralIntType old_start = old_starts[i];
                    LiteralIntType old_sz    = old_levels[i];
                    LiteralIntType new_level_index = i + 1; // destination level index
                    LiteralIntType new_start = new_starts[new_level_index];
                    

                    std::copy_n(data_.begin() + old_start, old_sz, new_data.begin() + new_start);
                }

                // recompute only the new top internal layer (level 0) from its children (level 1)
                // top-level nodes occupy global indices new_starts[0] .. new_starts[0]+new_levels[0]-1
                // their first child indices can be computed with first_child_of(parent_index)
                LiteralIntType top_count = new_levels[0];
                LiteralIntType top_start = new_starts[0];   // usually 0
                for (LiteralIntType j = 0; j < top_count; ++j) {
                    LiteralIntType parent_idx = top_start + j;
                    // first child in global indexing:
                    LiteralIntType first_child = base_tree::first_child_of(parent_idx);
                    Real sum = Real(0);
                    for (LiteralIntType c = 0; c < fanout; ++c) {
                        LiteralIntType child = first_child + c;
                        if (child >= new_total_nodes) break;
                        sum += new_data[child];
                    }
                    new_data[parent_idx] = sum;
                }
                // commit!
                num_layers_++;
                data_.swap(new_data);

            }
            else if (new_leaf_count+leaf_start_ >= base_tree::size()) {
                base_tree::resize(std::min(new_leaf_count*2,leaf_start_+max_leaf_));
            }
        }

        LiteralIntType max_leaf_;
        LiteralIntType leaf_end_;   // number of leaves requested by user
        typename std::make_signed<LiteralIntType>::type leaf_start_; // index of first leaf in data_
        LiteralIntType num_layers_;
        Real total_weight_ = 0;

        template< class UnaryOperation >
        static std::vector<Real> weight_distribution(std::size_t count, double xmin, double xmax,UnaryOperation unary_op ){
            std::vector<Real> distro;
            if (count<=0){
                distro.push_back(1);
            }
            else{
                double delta = (xmax - xmin)/count;
                for (IntType i=0; i<count;i++){
                    distro.push_back(unary_op(xmin + delta * (i+0.5)));
                }
            }
            return distro;
        }


        friend class dynamic_discrete_distribution;
    };
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    using param_type = Param;

    /**
     * @brief constructs a distribution with a single element 0 with a weight of 1. 
     */
    dynamic_discrete_distribution() : dynamic_discrete_distribution({1.0}) {}

    /**
     * @brief constructs a distribution with weights equal to the value in `weights` at each index.
     */
    explicit dynamic_discrete_distribution(const std::vector<Real>& weights)
        : dynamic_discrete_distribution(weights.begin(), weights.end()) {}
  
    /**
     * @brief constructs a distribution with the parameters in p. 
     */
    explicit dynamic_discrete_distribution(const Param& p){
        param_ = p;
    }

    /**
     * @brief constructs a distribution with the weights in the initializer_list `il`. Each weight will have the same index as it did in il. 
     */
    dynamic_discrete_distribution(const std::initializer_list<Real>& il)
        : dynamic_discrete_distribution(il.begin(), il.end()) {}
  
    /**
     * @brief Constructs the distribution with `count` weights that are generated using function `unary_op`. Each of the weights is equal to ${w_i} = unary_op(xmin + δ(i + 0.5))$, where $δ = (xmax − xmin)count$ and $i ∈ {0, ..., count − 1}$. `xmin` and `xmax` must be such that `δ > 0`. If `count == 0` the effects are the same as of the default constructor.
     */
    template< class UnaryOperation >
    dynamic_discrete_distribution( std::size_t count, double xmin, double xmax, UnaryOperation unary_op ){
        param_ = Param(count,xmin,xmax,unary_op);
    }

    /**
     * @brief constructs the distribution from the first and last iterators to a collection of weights of type `weight_type`. Each weight is indexed according to the number of elements between itself and first. 
     */
    template<class InputIt>
    dynamic_discrete_distribution(InputIt first, InputIt last){
        param_ = Param(first,last);
    }

    /**
     * @brief Resets the internal state of the distribution object. After a call to this function, the next call to operator() on the distribution object will not be dependent on previous calls to operator(). The distribution still depends on past calls to update_weight, push, and pop. (Note: in this implementation there is no need for reset, so it does no work. It is present for compatibility reasons)
     */
    void reset() {}

    
    /**
     * @brief Generates random numbers distributed according to the weights in the parameter set `param`. If it does not contain at least 1 weight, the behavior is undefined
     */
    template<class URNG>
    IntType operator()(URNG& g,const Param& param) const {
        Real total = param.total_weight();
        assert(total>0 && param.leaf_end_>0);
        if (total <= Real(0)) return 0;
        Real target = boost::random::generate_canonical<Real, Precision, URNG>(g) * total;
        if (target == Real(0)) return 0;
        typename Param::pos_type first_child = 0;

        // Start at the top internal node (index 0)
        typename Param::pos_type node = 0;

        for(LiteralIntType i=0; i<param.num_layers_-1;i++){
            Real cumulative = 0;
            bool chosen = false;
            for (LiteralIntType c = 0; c < fanout; ++c) {
                typename Param::pos_type child = first_child + c;
                Real w = param.weightsum_of(child);
                if (target < cumulative + w) {
                    node = child;
                    target -=cumulative; 
                    chosen = true;

                    break;
                }
                cumulative += w;
            }
            if (chosen == false){
                node = first_child;
                target -= cumulative;
                target +=param.weightsum_of(first_child);
            }

            first_child = param.base_tree::first_child_of(node); // first child in array
            
        }
        // last iteration shaved off for extra safety checks due to the possibility of floating point errors
        if (first_child>= param.leaf_start_+param.leaf_end_){ //floating point errors made selection move into empty part of tree at some point
            return this->operator()(g,param); //run selection algorithm again (probability of floating point errors causing this is quite small)
        }
        Real cumulative = 0;
        bool chosen = false;
        for (LiteralIntType c = 0; (c < fanout)&&(c+first_child<param.leaf_end_+param.leaf_start_); ++c) {
            typename Param::pos_type child = first_child + c;
            if (child >= param.leaf_end_+param.leaf_start_) break; 
            Real w = param.weightsum_of(child);
            if (target < cumulative + w) {
                node = child;
                target -=cumulative; 
                chosen = true;

                break;
            }
            cumulative += w;
        }
        if (chosen == false){
            node = first_child;
            target -= cumulative;
            target +=param.weightsum_of(first_child);
        }

            // otherwise, node has been updated to chosen_child
        

        // node is now a leaf
        return static_cast<IntType>(node - param.leaf_start_);
    
    }
    /**
     * @brief Generates random numbers that are distributed according to the weights in the distribution's parameter set. If it does not contain at least 1 weight, the behavior is undefined
     */
    template<class URNG>
    result_type operator()(URNG& g) const {
        return this->operator()(g,param_); 
    }

    /**
     * @brief returns a vector of probabilities of each integer that could be generated by this distribution using its associated parameter set. 
     */
    std::vector<Real> probabilities() const {
        return param_.probabilities();
    }

    /**
     * @brief gets the distribution's parameter set
     */
    Param param() const {
        return param_;
    }

    /**
     * @brief sets the distribution's parameter set to `p`
     */
    void param(const Param& p) { 
        param_ = p; 
    }

    /**
     * @brief returns the minimum integer that could be generated by the operator called on the distribution's parameter set assuming that it is not empty. This minimum will always be 0. 
     */
    static constexpr result_type min() { return 0; }

    /**
     * @brief returns the maximum integer that could be generated by the operator called on the distribution's parameter set assuming that it is not empty. 
     */
    result_type max() const { return param_.max(); }

    /**
     * @brief updates weight of int`i` in the distribution's parameter set to `new_weight`. If i is not in the distribution's parameter set, the behavior is undefined. 
     */
    void update_weight(IntType i, Real new_weight) {
        param_.update_weight(i, new_weight);
    }

    /**
     * @brief gets the weight of int `i` in the distribution's parameter set. If i is not in the parameter set, the behavior is undefined. 
     */
    Real get_weight(IntType i) const {
        return param_.get_weight_literal(i);
    }

    /**
     * @brief returns the number of weights in the distribution's parameter set. 
     */
    IntType size() const { return param_.size(); }

    /**
     * @brief returns the total of all of the weights in the distribution's parameter set.
     */
    Real total_weight() const noexcept {
        return param_.total_weight();
    }

    /**
     * @brief adds weights.size() elements to the end of the distribution's parameter set with weights given by `weights` vector. 
     */
    void push_back(const std::vector<Real>& weights){
        param_.push_back(weights);
    }

    /**
     * @brief adds an element to the end of distribution's parameter set with weight `weight`.
     */
    void push_back(Real weight) {
        param_.push_back(weight);
    }

    /**
     * @brief removes the `count` highest integers from the distribution's parameter set. If the distribution's parameter set is empty, this function's behavior is undefined. NOTE: in this implementation, this does not deallocate memory or decrease the depth of the underlying tree based data structure. For the purposes of asymptotic analysis, consider "N" to be the largest number of elements ever held in the distribution's parameter set. 
     */
    void pop_back(IntType count) {
        param_.pop_back(count);
    }

    /**
     * @brief removes the highest integer from the distribution's parameter set. If the distribution's parameter set is empty, this function's behavior is undefined. NOTE: in this implementation, this does not deallocate memory or decrease the depth of the underlying tree based data structure. For the purposes of asymptotic analysis, consider "N" to be the largest number of elements ever held in the distribution's parameter set. 
     */
    void pop_back() {
        param_.pop_back();
    }

    /**
     * @brief compares the parameter sets of `rhs` and `lhs` for equality
     */
    BOOST_RANDOM_DETAIL_EQUALITY_OPERATOR(This, lhs, rhs){
        return (rhs.param_ == lhs.param_);
    }
    
    /**
     * @brief compares the parameter sets of `rhs` and `lhs` for inequality
     */
    BOOST_RANDOM_DETAIL_INEQUALITY_OPERATOR(This)
    
    
    /**
     * @brief Restores the distribution parameters with data read from `stream`. The formatting flags of `stream` are unchanged. The data must have been written using a stream with the same locale, `CharT` and `Traits` template parameters, otherwise the behavior is undefined. If bad input is encountered, stream.setstate(std::ios::failbit) is called, which may throw std::ios_base::failure. `dist` is unchanged in that case.
     */
    BOOST_RANDOM_DETAIL_ISTREAM_OPERATOR(stream, This, dist){
        return (stream>>dist.param_);
    }

    /**
     * @brief Writes a textual representation of the distribution parameters to `stream`. The formatting flags and fill character of `stream` are unchanged.
     */
    BOOST_RANDOM_DETAIL_OSTREAM_OPERATOR(stream, This, dist){        
        return (stream<<dist.param_);
    }

private:
    Param param_;
    static constexpr LiteralIntType fanout = Fanout; 

};

}}
#include <boost/random/detail/enable_warnings.hpp>

#endif