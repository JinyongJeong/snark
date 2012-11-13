// This file is part of snark, a generic and flexible library
// for robotics research.
//
// Copyright (C) 2011 The University of Sydney
//
// snark is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// snark is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with snark. If not, see <http://www.gnu.org/licenses/>.

/// @author Cedric Wohlleber

#ifndef SNARK_MATH_INTERVAL_H_
#define SNARK_MATH_INTERVAL_H_

#include <boost/optional.hpp>
#include <comma/base/exception.h>
#include <comma/math/compare.h>
#include <Eigen/Core>

namespace snark { namespace math {

/// multi-dimensional interval
template < typename T, int N >
class closed_interval
{
    public:
        typedef Eigen::Matrix< T, N, 1 > vector_type;

        /// constructor
        closed_interval( const vector_type& min, const vector_type& max ) : m_interval( std::make_pair( get_min( min, max ), get_max( min, max ) ) ) { if( !less_or_equal( min, max ) ) { COMMA_THROW( comma::exception, "invalid interval" ); } }

        /// constructor
        closed_interval( const vector_type& rhs ) : m_interval( std::make_pair( rhs, rhs ) ) {}
        
        /// default constructor
        closed_interval() {} //interval(): m_interval( std::make_pair( vector_type::Zero(), vector_type::Zero() ) ) {}

        /// return value
        const std::pair< vector_type, vector_type >& operator()() const { assert( m_interval ); return m_interval; }

        /// return left boundary (convenience method)
        const vector_type& min() const { assert( m_interval ); return m_interval->first; }

        /// return right boundary (convenience method)
        const vector_type& max() const { assert( m_interval ); return m_interval->second; }

        /// return true, if variable belongs to the interval
        bool contains( const vector_type& rhs ) const { assert( m_interval ); return less_or_equal( m_interval->first, rhs ) && less_or_equal( rhs, m_interval->second ); }

        /// return true, if the whole rhs interval belongs to the interval
        bool contains( const closed_interval& rhs ) const { assert( m_interval && rhs ); return contains( rhs.min() ) && contains( rhs.max() ); }

        /// compute the hull of the interval and [rhs]
        closed_interval< T, N > hull( const vector_type& rhs ) const { return closed_interval( get_min( m_interval->first, rhs ), get_max( m_interval->second, rhs ) ); }

        /// compute the hull of 2 intervals
        closed_interval< T, N > hull( const closed_interval& rhs ) const { return closed_interval( get_min( m_interval->first, rhs.min() ), get_max( m_interval->second, rhs.max() ) ); }
        
        /// compute and assign the hull of the interval and [rhs]
        const closed_interval< T, N >& set_hull( const vector_type& rhs ) { *this = m_interval ? hull( rhs ) : closed_interval( rhs ); return *this; }

        /// compute and assign the hull of 2 intervals
        const closed_interval< T, N >& set_hull( const closed_interval& rhs ) { *this = m_interval ? hull( rhs ) : closed_interval( rhs ); return *this; }

        /// equality
        bool operator==( const closed_interval& rhs ) const { assert( m_interval && rhs ); return m_interval->first.isApprox( rhs().first ) && m_interval->second.isApprox( rhs().second ); }

        /// unequality
        bool operator!=( const closed_interval& rhs ) const { assert( m_interval && rhs ); return !operator==( rhs ); }

    private:
        // todo: use epsilon from <limits> for comparison
        static bool less_or_equal( const vector_type& lhs, const vector_type& rhs ) { return ( ( lhs.array() <= rhs.array() ).all() ); }
        static vector_type get_min( const vector_type& lhs, const vector_type& rhs ) { return rhs.array().min( lhs.array() ); }
        static vector_type get_max( const vector_type& lhs, const vector_type& rhs ) { return rhs.array().max( lhs.array() ); }
        
        boost::optional< std::pair< vector_type, vector_type > > m_interval;
};

} } // namespace snark { namespace math {

#endif // SNARK_MATH_INTERVAL_H_
