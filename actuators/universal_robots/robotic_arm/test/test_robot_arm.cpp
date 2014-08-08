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
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the The University of Sydney.
// 4. Neither the name of the The University of Sydney nor the
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

#include <gtest/gtest.h>
#include <comma/csv/stream.h>
#include <comma/csv/ascii.h>
#include <comma/visiting/traits.h>
#include <comma/base/types.h>
#include <comma/visiting/apply.h>
#include <comma/name_value/ptree.h>
#include <comma/csv/stream.h>
#include <comma/name_value/parser.h>
#include <comma/io/stream.h>
#include <comma/math/compare.h>
#include "../traits.h"
#include "../ur5/transforms/transforms.h"
#include <boost/property_tree/json_parser.hpp>


namespace snark { namespace ur { namespace robotic_arm {

template < typename T >
comma::csv::ascii< T >& ascii() { 
    static comma::csv::ascii< T > ascii_;
    return ascii_;
}


TEST( robot_arm, robot_arm_config )
{
    config cfg;
    continuum_t& continuum = cfg.continuum;
    continuum.home_position[0] = 1.11;
    continuum.home_position[1] = 2.22322;
    continuum.home_position[5] = 6.0;

    std::string line;
    EXPECT_EQ( "1.11,2.22322,0,0,0,6,\"\"", ascii< config >().put( cfg, line ) );

    const std::string expected= "{\"continuum\":{\"home_position\":{\"0\":\"1.11\",\"1\":\"2.22322\",\"2\":\"0\",\"3\":\"0\",\"4\":\"0\",\"5\":\"6\"},\"work_directory\":\"\"}}\n";
    {
        std::ostringstream oss;
        boost::property_tree::ptree t;
        comma::to_ptree to_ptree( t );
        comma::visiting::apply( to_ptree ).to( cfg );
        boost::property_tree::write_json( oss, t, false );    
        EXPECT_EQ( expected, oss.str() );
    }
    {
        std::istringstream iss( expected );
        boost::property_tree::ptree t;
        comma::from_ptree( t, true );
        boost::property_tree::read_json( iss, t );
        comma::from_ptree from_ptree( t, true );
        config conf;
        comma::visiting::apply( from_ptree ).to( conf );

        EXPECT_EQ( continuum.home_position[0], conf.continuum.home_position[0] );
        EXPECT_EQ( continuum.home_position[1], conf.continuum.home_position[1] );
//         EXPECT_EQ( continuum, conf.continuum );
    }
}

TEST( robot_arm, ur5_transforms )
{
    snark::applications::position tcp, laser;
    
    /// Home position
    {
        boost::array< plane_angle_t, joints_num > angles;
        angles[0] = -2.351464889560617e-05 * radian;
        angles[1] = -1.395978507479843 * radian;
        angles[2] = -2.610982070984108 * radian;
        angles[3] = 0.8658186400879389 * radian;
        angles[4] = 1.5710028270451 * radian;
        angles[5] = -1.994146684580755e-05 * radian;
        
        ur5::tcp_transform( angles, tcp, laser );
        
        static const double epsilon = 0.0001;
        EXPECT_TRUE( comma::math::equal( tcp.coordinates.x(), 0.2621920341712425, epsilon) );
        EXPECT_TRUE( comma::math::equal( tcp.coordinates.y(), -0.1089892323633621, epsilon) );
        EXPECT_TRUE( comma::math::equal( tcp.coordinates.z(), 0.3041163274456367, epsilon) );
        EXPECT_TRUE( comma::math::equal( tcp.orientation.x(), 0.0001829856036957604, epsilon) );
        EXPECT_TRUE( comma::math::equal( tcp.orientation.y(), 0.0004507152117164778, epsilon) );
        EXPECT_TRUE( comma::math::equal( laser.coordinates.x(), 0.2621920341712425, epsilon) );
        EXPECT_TRUE( comma::math::equal( laser.coordinates.y(), -0.1089892323633621, epsilon) );
        EXPECT_TRUE( comma::math::equal( laser.coordinates.z(), 0.4141163162506301, epsilon) );
    }
}


} } } // namespace snark { namespace ur { namespace robotic_arm {

int main( int argc, char* argv[] )
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    return 0;
}
