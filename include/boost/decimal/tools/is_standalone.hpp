//  Copyright Matt Borland 2022.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DECIMAL_TOOLS_IS_STANDALONE_HPP
#define BOOST_DECIMAL_TOOLS_IS_STANDALONE_HPP

// If any of the required dependencies are missing fallback to standalone mode
#if !__has_include(<boost/config.hpp>) || !__has_include(<boost/endian.hpp>) || !__has_include(<boost/type_traits.hpp>)
#   ifndef BOOST_DECIMAL_STANDALONE
#       define BOOST_DECIMAL_STANDALONE
#   endif
#endif

#endif // BOOST_DECIMAL_TOOLS_IS_STANDALONE_HPP
