/*
 *  gtest-varstorage_test.cpp
 *  xBeeHome
 *
 *  Created by Justin Hammond on 11/4/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#include <gtest/gtest.h>
#include "include/varcontainer.hpp"
#include "src/varcontainer.cpp"

namespace testing {
	namespace internal {
		namespace {
			class VarContainerTest : public testing::Test {
				protected:
				     virtual void SetUp() {
						 this->Vars = new VarStorage();
						 this->Vars->addValue("INT", (int)123);
						 this->Vars->addValue("STRING", (std::string)"String");
						 this->Vars->addValue("LONG", (long)-1);
						 this->Vars->addValue("LONGLONG", (long long)-1);
						 this->Vars->addValue("FLOAT", (float)1.24);
						 this->Vars->addBValue("BOOL", (bool)true);
						 this->Vars->addValue("DATE", boost::posix_time::ptime(boost::posix_time::time_from_string("2010-01-10 10:23:23")));

					 }
				
				VarStorage *Vars;
			};
			
			
			TEST_F(VarContainerTest, ReturnsTrueFor7Items) {
				EXPECT_TRUE(Vars->getSize() == 7);
			}
			
			TEST_F(VarContainerTest, CheckIntContents) {
				int i;
				Vars->getValue("INT", i);
				EXPECT_EQ(123,i); 
			}
			TEST_F(VarContainerTest, CheckStringContents) {
				std::string val;
				Vars->getValue("STRING", val);
				EXPECT_STREQ("String", val.c_str());
			}
			TEST_F(VarContainerTest, CheckLongContents) {
				long i;
				Vars->getValue("LONG", i);
				EXPECT_EQ((long)-1, i);
			}
			TEST_F(VarContainerTest, CheckLongLongContents) {
				long long i;
				Vars->getValue("LONGLONG", i);
				EXPECT_EQ((long long)-1, i);
			}
			TEST_F(VarContainerTest, CheckFloatContents) {
				float i;
				Vars->getValue("FLOAT", i);
				EXPECT_FLOAT_EQ(1.24, i);
			}
			TEST_F(VarContainerTest, CheckBoolContents) {
				bool i;
				Vars->getValue("BOOL", i);
				EXPECT_EQ(true, i);
			}
			TEST_F(VarContainerTest, CheckDateContents) {
				boost::posix_time::ptime i;
				Vars->getValue("DATE", i);
				EXPECT_EQ(boost::posix_time::ptime(boost::posix_time::time_from_string("2010-01-10 10:23:23")), i);
			}
			TEST_F(VarContainerTest, AddIntValueAndCheck) {
				int i = 321;
				EXPECT_TRUE(Vars->addValue("INT", i)) << "Adding Int Value Failed";
				int j;
				EXPECT_TRUE(Vars->getValue("INT", j, 1)) << "Getting 2nd Int Value Failed";
				EXPECT_EQ((int)321, j) << "Int Value Didn't Match";
			}
			TEST_F(VarContainerTest, AddStringValueAndCheck) {
				std::string val("Testing213");
				EXPECT_TRUE(Vars->addValue("STRING", val)) << "Adding String Value Failed";
				std::string val2;
				EXPECT_TRUE(Vars->getValue("STRING", val2, 1)) << "Getting 2nd Int Value Failed";
				EXPECT_STREQ("Testing213", val.c_str()) << "String Value Didn't Match";
			}
			TEST_F(VarContainerTest, AddLongValueAndCheck) {
				long i = (long)-2;
				EXPECT_TRUE(Vars->addValue("LONG", i)) << "Adding Long Value Failed";
				long j;
				EXPECT_TRUE(Vars->getValue("LONG", j, 1)) << "Getting 2nd Long Value Failed";
				EXPECT_EQ((long)-2, j) << "Long Value Didn't Match";
			}
			TEST_F(VarContainerTest, AddLongLongValueAndCheck) {
				long long i = (long long)-2;
				EXPECT_TRUE(Vars->addValue("LONGLONG", i)) << "Adding Long Long Value Failed";
				long long j;
				EXPECT_TRUE(Vars->getValue("LONGLONG", j, 1)) << "Getting 2nd Long Long Value Failed";
				EXPECT_EQ((long long)-2, j) << "long long Value Didn't Match";
			}
			TEST_F(VarContainerTest, AddFloatValueAndCheck) {
				float i = 9.32;
				EXPECT_TRUE(Vars->addValue("FLOAT", i)) << "Adding float Value Failed";
				float j;
				EXPECT_TRUE(Vars->getValue("FLOAT", j, 1)) << "Getting 2nd Float Value Failed";
				EXPECT_FLOAT_EQ(9.32, j) << "float Value Didn't Match";
			}
			TEST_F(VarContainerTest, AddBOOLValueAndCheck) {
				bool i = false;
				EXPECT_TRUE(Vars->addBValue("BOOL", i)) << "Adding Bool Value Failed";
				bool j;
				EXPECT_TRUE(Vars->getValue("BOOL", j, 1)) << "Getting 2nd Bool Value Failed";
				EXPECT_EQ(false, j) << "Bool Value Didn't Match";
			}
			TEST_F(VarContainerTest, AddDateValueAndCheck) {
				boost::posix_time::ptime i(boost::posix_time::time_from_string("2011-04-12 18:54:34"));
				EXPECT_TRUE(Vars->addValue("DATE", i));
				boost::posix_time::ptime j;
				EXPECT_TRUE(Vars->getValue("DATE", j, 1));
				EXPECT_EQ(i, j);
			}
			TEST_F(VarContainerTest, ReplaceIntValueAndCheck) {
				int i = 456;
				EXPECT_TRUE(Vars->addValue("INT", i)) << "Adding 2nd Value Failed";
				int j;
				EXPECT_TRUE(Vars->getValue("INT", j, 1)) << "Getting 2nd Value Failed";
				EXPECT_EQ(i, j) << "2nd Value didn't match";
				i = 789;
				EXPECT_TRUE(Vars->replaceValue("INT", i, 1)) << "Replace Failed";
				EXPECT_TRUE(Vars->getValue("INT", j, 1)) << "Getting Replaced Value Failed";
				EXPECT_EQ(i, j) << "Replacement Value didn't match";
				EXPECT_TRUE(Vars->getValue("INT", j, 0)) << "Getting First Value Failed";
				EXPECT_EQ(123, j);
			}
			TEST_F(VarContainerTest, ReplaceStringValueAndCheck) {
				std::string val("InitialString");
				EXPECT_TRUE(Vars->addValue("STRING", val)) << "Adding 2nd Value Failed";
				std::string j;
				EXPECT_TRUE(Vars->getValue("STRING", j, 1)) << "Getting 2nd Value Failed";
				EXPECT_STREQ(val.c_str(), j.c_str()) << "2nd Value didn't match";
				val = "789";
				EXPECT_TRUE(Vars->replaceValue("STRING", val, 1)) << "Replace Failed";
				EXPECT_TRUE(Vars->getValue("STRING", j, 1)) << "Getting Replaced Value Failed";
				EXPECT_STREQ(val.c_str(), j.c_str()) << "Replacement Value didn't match";
				EXPECT_TRUE(Vars->getValue("STRING", j, 0)) << "Getting First Value Failed";
				EXPECT_STREQ("String", j.c_str());
			}				
			TEST_F(VarContainerTest, ReplaceLongValueAndCheck) {
				long i = (long)-456;
				EXPECT_TRUE(Vars->addValue("LONG", i)) << "Adding 2nd Value Failed";
				long j;
				EXPECT_TRUE(Vars->getValue("LONG", j, 1)) << "Getting 2nd Value Failed";
				EXPECT_EQ(i, j) << "2nd Value didn't match";
				i = (long)-789;
				EXPECT_TRUE(Vars->replaceValue("LONG", i, 1)) << "Replace Failed";
				EXPECT_TRUE(Vars->getValue("LONG", j, 1)) << "Getting Replaced Value Failed";
				EXPECT_EQ(i, j) << "Replacement Value didn't match";
				EXPECT_TRUE(Vars->getValue("LONG", j, 0)) << "Getting First Value Failed";
				EXPECT_EQ((long)-1, j);
			}
			TEST_F(VarContainerTest, ReplaceLongLongValueAndCheck) {
				long long i = (long long)-325;
				EXPECT_TRUE(Vars->addValue("LONGLONG", i)) << "Adding 2nd Value Failed";
				long long j;
				EXPECT_TRUE(Vars->getValue("LONGLONG", j, 1)) << "Getting 2nd Value Failed";
				EXPECT_EQ(i, j) << "2nd Value didn't match";
				i = (long long)-729;
				EXPECT_TRUE(Vars->replaceValue("LONGLONG", i, 1)) << "Replace Failed";
				EXPECT_TRUE(Vars->getValue("LONGLONG", j, 1)) << "Getting Replaced Value Failed";
				EXPECT_EQ(i, j) << "Replacement Value didn't match";
				EXPECT_TRUE(Vars->getValue("LONGLONG", j, 0)) << "Getting First Value Failed";
				EXPECT_EQ((long)-1, j);
			}
			TEST_F(VarContainerTest, ReplaceFloatValueAndCheck) {
				float i = 22.341;
				EXPECT_TRUE(Vars->addValue("FLOAT", i)) << "Adding 2nd Value Failed";
				float j;
				EXPECT_TRUE(Vars->getValue("FLOAT", j, 1)) << "Getting 2nd Value Failed";
				EXPECT_FLOAT_EQ(i, j) << "2nd Value didn't match";
				i = 11.932;
				EXPECT_TRUE(Vars->replaceValue("FLOAT", i, 1)) << "Replace Failed";
				EXPECT_TRUE(Vars->getValue("FLOAT", j, 1)) << "Getting Replaced Value Failed";
				EXPECT_FLOAT_EQ(i, j) << "Replacement Value didn't match";
				EXPECT_TRUE(Vars->getValue("FLOAT", j, 0)) << "Getting First Value Failed";
				EXPECT_FLOAT_EQ(1.24, j);
			}			
			TEST_F(VarContainerTest, ReplaceBoolValueAndCheck) {
				bool i = true;
				EXPECT_TRUE(Vars->addBValue("BOOL", i)) << "Adding 2nd Value Failed";
				bool j;
				EXPECT_TRUE(Vars->getValue("BOOL", j, 1)) << "Getting 2nd Value Failed";
				EXPECT_EQ(i, j) << "2nd Value didn't match";
				i = false;
				EXPECT_TRUE(Vars->replaceBValue("BOOL", i, 1)) << "Replace Failed";
				EXPECT_TRUE(Vars->getValue("BOOL", j, 1)) << "Getting Replaced Value Failed";
				EXPECT_EQ(i, j) << "Replacement Value didn't match";
				EXPECT_TRUE(Vars->getValue("BOOL", j, 0)) << "Getting First Value Failed";
				EXPECT_EQ(true, j);
			}
			TEST_F(VarContainerTest, ReplaceDateValueAndCheck) {
				boost::posix_time::ptime i(boost::posix_time::time_from_string("2011-04-12 18:54:34"));
				EXPECT_TRUE(Vars->addValue("DATE", i));
				boost::posix_time::ptime j;
				EXPECT_TRUE(Vars->getValue("DATE", j, 1));
				EXPECT_EQ(i, j);
				i = boost::posix_time::time_from_string("2012-12-12 23:59:59");
				EXPECT_TRUE(Vars->replaceValue("DATE", i, 1));
				EXPECT_TRUE(Vars->getValue("DATE", j, 1));
				EXPECT_EQ(i, j);
				EXPECT_TRUE(Vars->getValue("DATE", j, 0));
				EXPECT_EQ(boost::posix_time::time_from_string("2010-01-10 10:23:23"), j);
			}				
			TEST_F(VarContainerTest, AddInvalidTypeToContainer) {
				std::string val("invalid");
				EXPECT_EQ(-1, Vars->addValue("INT", val));
				EXPECT_EQ(-1, Vars->addValue("LONG", val));
				EXPECT_EQ(-1, Vars->addValue("LONGLONG", val));
				EXPECT_EQ(-1, Vars->addValue("FLOAT", val));
				EXPECT_EQ(-1, Vars->addValue("BOOL", val));
				EXPECT_EQ(-1, Vars->addValue("DATE", val));
				long i = (long)-432;
				EXPECT_EQ(-1, Vars->addValue("STRING", i));
				EXPECT_EQ(-1, Vars->addValue("INT", i));
				EXPECT_EQ(-1, Vars->addValue("LONGLONG", i));
				EXPECT_EQ(-1, Vars->addValue("FLOAT", i));
				EXPECT_EQ(-1, Vars->addValue("BOOL", i));
				EXPECT_EQ(-1, Vars->addValue("DATE", i));
			}
			
			
		}  // namespace
	}  // namespace internal
}  // namespace testing
