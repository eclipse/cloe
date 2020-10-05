/*
 * Copyright 2020 Robert Bosch GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * \file rdb_transceiver_tcp_test.cpp
 * \see  rdb_transceiver_tcp.hpp
 */

#include <iostream>  // for cout
#include <vector>    // for vector<>

#include <boost/asio.hpp>  // for error_code, endpoint, io_service, ...

#include <gtest/gtest.h>  // for TEST, ASSERT_ANY_THROW, ...

#include "rdb_transceiver_tcp.hpp"  // for RdbTransceiverTcp

void handle_write(const boost::system::error_code& error, size_t /* bytes_transferred */) {
  std::cout << "write code: " << error << std::endl;
}

void accept_handler(const boost::system::error_code& error) {
  std::cout << "accept code: " << error << std::endl;
  if (!error) {
    // Accept succeeded.
  }
}

TEST(vtd_rdb_transceiver_tcp_test, open) {
  ASSERT_ANY_THROW(vtd::RdbTransceiverTcp("localhost", 1));
}

TEST(vtd_rdb_transceiver_tcp_test, connect) {
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::acceptor acceptor(
      io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 59140));
  boost::asio::ip::tcp::socket socket(io_service);
  vtd::RdbTransceiverTcp transceiver("localhost", 59140);
}
