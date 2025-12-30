/*-------------------------------------------------------------------------
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 * CException_test.cc
 *
 * IDENTIFICATION
 *	  contrib/pax_storage/src/cpp/exceptions/CException_test.cc
 *
 *-------------------------------------------------------------------------
 */


#include "CException.h"
#include "comm/cbdb_wrappers.h"
#include "comm/gtest_wrappers.h"

#ifndef ENABLE_DEBUG
#error "CException_test.cc must be compiled with ENABLE_DEBUG"
#endif

namespace cbdb {
extern void StackTrace(char *stack_buffer);
}
namespace pax::tests {
class TestCException {
public:
  void printStack() {
    cbdb::StackTrace(stack_);
    printf("Stack trace:\n%s", stack_);
  }
  cbdb::CException::ExType invoke() {
    CBDB_TRY();
    {
      throw cbdb::CException(cbdb::CException::ExType::kExTypeIOError);
    }
    CBDB_CATCH_MATCH(cbdb::CException &ex);
    {
      return ex.EType();
    }
    CBDB_END_TRY();
  }

  char stack_[DEFAULT_STACK_MAX_SIZE];
};

TEST(CExceptionTest, TestStackTrace) {
  TestCException ex;
  ex.printStack();
}

TEST(CExceptionTest, TestRaiseAndReraise) {
  TestCException ex;
  bool run_caught = false;

  auto etype = ex.invoke();
  EXPECT_EQ(etype, cbdb::CException::ExType::kExTypeIOError);

  try {
    CBDB_TRY();
    {
      CBDB_RAISE(cbdb::CException::ExType::kExTypeAbort, "Test raise exception");
    }
    CBDB_CATCH_MATCH(cbdb::CException &ex);
    {
      run_caught = true;
      EXPECT_EQ(ex.EType(), cbdb::CException::ExType::kExTypeAbort);
      EXPECT_NE(std::string(ex.What()).find("Test raise exception"), std::string::npos);
      CBDB_RERAISE(ex);
    }
    CBDB_END_TRY();
  } catch (cbdb::CException &ex) {
    EXPECT_EQ(ex.EType(), cbdb::CException::ExType::kExTypeAbort);
    EXPECT_NE(std::string(ex.What()).find("Test raise exception"), std::string::npos);
  }

  EXPECT_TRUE(run_caught);
}

TEST(CExceptionTest, TestFinally) {
  TestCException ex;
  bool finally_executed = false;
  try {
    CBDB_TRY();
    {
      (void) ex.printStack();
    }
    CBDB_CATCH_DEFAULT();
    CBDB_FINALLY(
      finally_executed = true;
    );
    CBDB_END_TRY();
  } catch (...) {
  }
  EXPECT_EQ(finally_executed, true);


  finally_executed = false;
  bool pg_caught = false;
  PG_TRY();
  {
    CBDB_TRY();
    {
      CBDB_RAISE(cbdb::CException::ExType::kExTypeAbort, "Test finally with raise");
    }
    CBDB_CATCH_DEFAULT();
    CBDB_FINALLY(
      finally_executed = true;
    );
    CBDB_END_TRY();
  }
  PG_CATCH();
  {
    pg_caught = true;
    FlushErrorState();
  }
  PG_END_TRY();

  EXPECT_EQ(finally_executed, true);
  EXPECT_EQ(pg_caught, true);
}

TEST(CExceptionTest, TestAppendDetailMessage) {
  cbdb::CException ex("test_file.cc", 123, cbdb::CException::ExType::kExTypeLogicError, "Initial message. ");
  ex.AppendDetailMessage("Appended message part 1. ");
  ex.AppendDetailMessage(std::string("Appended message part 2."));

  std::string what = ex.What();
  EXPECT_NE(what.find("Initial message."), std::string::npos);
  EXPECT_NE(what.find("Appended message part 1."), std::string::npos);
  EXPECT_NE(what.find("Appended message part 2."), std::string::npos);
}

} // namespace pax::tests