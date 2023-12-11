#include <iostream>
#include <string>

#include <google/protobuf/stubs/common.h>

#include "addressbook.pb.h"

using namespace std;

int main() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  tutorial::AddressBook address_book;
  tutorial::Person* person = address_book.add_person();

  person->set_id(123);
  person->set_name("John");
  person->set_email("john@gmail.com");
  tutorial::Person::PhoneNumber* phone_number = person->add_phone();
  phone_number->set_number("1234567");
  phone_number->set_type(tutorial::Person::MOBILE);

  string output;
  address_book.SerializeToString(&output);

  tutorial::AddressBook address_book2;
  address_book2.ParseFromString(output);

  cout<<address_book2.DebugString();

  return 0;
}
