// This file is part of snark, a generic and flexible library for robotics research
// Copyright (c) 2011 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "../../../../imaging/cv_mat/detail/bitwise.h"

#include <gtest/gtest.h>

#include <boost/assign.hpp>

#include <iostream>

using namespace snark::cv_mat;
using namespace snark::cv_mat::bitwise;
using boost::spirit::ascii::space;

namespace {

    struct writer
    {
        typedef boost::static_visitor< boost::function< std::ostream & ( std::ostream & ) > >::result_type result_type;

        result_type term( const std::string & s ) const { return [ &s ]( std::ostream & os ) -> std::ostream & { os << s; return os; }; }
        result_type op_and( const result_type & opl, const result_type & opr ) const { return [ opl, opr ]( std::ostream & os ) -> std::ostream & { os << '('; opl( os ); os << " & "; opr( os ); os << ')'; return os; }; }
        result_type op_or(  const result_type & opl, const result_type & opr ) const { return [ opl, opr ]( std::ostream & os ) -> std::ostream & { os << '('; opl( os ); os << " | "; opr( os ); os << ')'; return os; }; }
        result_type op_xor( const result_type & opl, const result_type & opr ) const { return [ opl, opr ]( std::ostream & os ) -> std::ostream & { os << '('; opl( os ); os << " ^ "; opr( os ); os << ')'; return os; }; }
        result_type op_not( const result_type & op ) const { return [ op ]( std::ostream & os ) -> std::ostream & { os << "(~"; op( os ); os << ')'; return os; }; }
    };

    template< typename T >
    using lookup_map_t = std::map< std::string, T >;

    template< typename T >
    struct logician
    {
        logician( const lookup_map_t< T > & m ) : m_( m ) {}
        const lookup_map_t< T > & m_;

        typedef typename boost::static_visitor< boost::function< T ( boost::none_t ) > >::result_type result_type;

        result_type term( const std::string & s ) const { return [ &s, m = m_ ]( boost::none_t ) -> T { return m.at( s ); }; }
        result_type op_and( const result_type & opl, const result_type & opr ) const { return [ opl, opr ]( boost::none_t ) -> T { const T & il = opl( boost::none ); const T & ir = opr( boost::none ); return il & ir; }; }
        result_type op_or(  const result_type & opl, const result_type & opr ) const { return [ opl, opr ]( boost::none_t ) -> T { const T & il = opl( boost::none ); const T & ir = opr( boost::none ); return il | ir; }; }
        result_type op_xor( const result_type & opl, const result_type & opr ) const { return [ opl, opr ]( boost::none_t ) -> T { const T & il = opl( boost::none ); const T & ir = opr( boost::none ); return il ^ ir; }; }
        result_type op_not( const result_type & op ) const { return [ op ]( boost::none_t ) -> T { const T & i = op( boost::none ); return ~i; }; }
    };

    const std::vector< std::string > inputs =
        {
            "a and b",
            "a or b",
            "a xor b",
            "not a",
            "not a and b",
            "not (a and b)",
            "a or b or c",
            "(a and b) xor ((c and d) or (a and b))",
            "a and b xor (c and d or a and b)",
        };
    const std::vector< std::string > expected =
        {
            "(a & b)",
            "(a | b)",
            "(a ^ b)",
            "(~a)",
            "((~a) & b)",
            "(~(a & b))",
            "(a | (b | c))",
            "((a & b) ^ ((c & d) | (a & b)))",
            "((a & b) ^ ((c & d) | (a & b)))",
        };
    const std::vector< boost::function< int( int, int, int, int ) > > direct =
        {
            []( int a, int b, int c, int d ) ->int { return (a & b); },
            []( int a, int b, int c, int d ) ->int { return (a | b); },
            []( int a, int b, int c, int d ) ->int { return (a ^ b); },
            []( int a, int b, int c, int d ) ->int { return (~a); },
            []( int a, int b, int c, int d ) ->int { return ((~a) & b); },
            []( int a, int b, int c, int d ) ->int { return (~(a & b)); },
            []( int a, int b, int c, int d ) ->int { return (a | (b | c)); },
            []( int a, int b, int c, int d ) ->int { return ((a & b) ^ ((c & d) | (a & b))); },
            []( int a, int b, int c, int d ) ->int { return ((a & b) ^ ((c & d) | (a & b))); },
        };

    template< typename T >
    int call_direct( const lookup_map_t< T > & m, const boost::function< T( T, T, T, T ) > & f ) { return f( m.at("a"), m.at("b"), m.at("c"), m.at("d") ); }

} // anonymous

TEST( bitwise, printer )
{
    for ( size_t i = 0; i < inputs.size(); ++i )
    {
        auto f( std::begin( inputs[i] ) ), l( std::end( inputs[i] ) );
        parser< decltype( f ) > p;

        expr result;
        bool ok = boost::spirit::qi::phrase_parse( f, l, p, boost::spirit::qi::space, result );
        EXPECT_TRUE( ok );
        EXPECT_EQ( f, l );
        {
            std::ostringstream os;
            os << result;
            EXPECT_EQ( os.str(), expected[i] );
        }
    }
}

TEST( bitwise, writer )
{
    for ( size_t i = 0; i < inputs.size(); ++i )
    {
        auto f( std::begin( inputs[i] ) ), l( std::end( inputs[i] ) );
        parser< decltype( f ) > p;

        expr result;
        bool ok = boost::spirit::qi::phrase_parse( f, l, p, boost::spirit::qi::space, result );
        EXPECT_TRUE( ok );
        EXPECT_EQ( f, l );
        {
            std::ostringstream os;
            writer w;
            auto scribe = boost::apply_visitor( visitor< std::ostream &, std::ostream &, writer >( w ), result );
            scribe( os );
            EXPECT_EQ( os.str(), expected[i] );
        }
    }
}

TEST( bitwise, logician )
{
    std::vector< lookup_map_t< int > > lookup_maps = {
        { { "a",  135 }, { "b",   84 }, { "c",  213 }, { "d",  104 } },
        { { "a",   13 }, { "b", 2983 }, { "c", -676 }, { "d", 9238 } },
        { { "a", 4567 }, { "b", -837 }, { "c", 9652 }, { "d",  -38 } },
    };
    for ( size_t i = 0; i < inputs.size(); ++i )
    {
        auto f( std::begin( inputs[i] ) ), l( std::end( inputs[i] ) );
        parser< decltype( f ) > p;

        expr result;
        bool ok = boost::spirit::qi::phrase_parse( f, l, p, boost::spirit::qi::space, result );
        EXPECT_TRUE( ok );
        EXPECT_EQ( f, l );
        {
            for ( const auto & m : lookup_maps )
            {
                logician< int > l( m );
                auto worker = boost::apply_visitor( visitor< boost::none_t, int, logician< int > >( l ), result );
                int r = worker( boost::none );
                int q = call_direct( m, direct[i] );
                EXPECT_EQ( r, q );
            }
        }
    }
}
