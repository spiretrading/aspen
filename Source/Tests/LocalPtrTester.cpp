#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <doctest/doctest.h>
#include "Aspen/LocalPtr.hpp"

using namespace Aspen;

namespace {
  struct Path {
    std::string m_value;

    Path()
      : m_value("default") {}

    explicit Path(int)
      : m_value("int") {}

    Path(int, int)
      : m_value("int, int") {}

    Path(const Path& path)
      : m_value("copy") {}

    Path(Path&)
      : m_value("conversion") {}

    Path(Path&& path) noexcept
      : m_value("move") {}

    Path& operator =(const Path& path) {
      m_value = "copy assignment";
      return *this;
    }

    Path& operator =(Path&& path) noexcept {
      m_value = "move assignment";
      return *this;
    }
  };

  struct Allocation {
    static inline auto count = 0;

    std::shared_ptr<int> m_value;

    Allocation()
      : m_value(std::make_shared<int>(0)) {
      ++count;
    }

    Allocation(const Allocation& allocation)
      : m_value(std::make_shared<int>(*allocation.m_value)) {
      ++count;
    }

    Allocation& operator =(const Allocation& allocation) {
      *m_value = *allocation.m_value;
      return *this;
    }
  };

  struct Throws {
    Throws() {}

    explicit Throws(int) noexcept {}

    Throws& operator =(int) {
      return *this;
    }

    Throws& operator =(double) noexcept {
      return *this;
    }
  };
}

TEST_SUITE("LocalPtr") {
  TEST_CASE("constructing_forwards_its_arguments") {
    REQUIRE(LocalPtr<Path>()->m_value == "default");
    REQUIRE(LocalPtr<Path>(1)->m_value == "int");
    REQUIRE(LocalPtr<Path>(1, 2)->m_value == "int, int");
    REQUIRE(*LocalPtr<std::string>(3, 'a') == "aaa");
  }

  TEST_CASE("copying_selects_the_copy_constructor") {
    auto pointer = LocalPtr<Path>();
    auto direct = LocalPtr<Path>(pointer);
    REQUIRE(direct->m_value == "copy");
    auto copy = pointer;
    REQUIRE(copy->m_value == "copy");
    const auto& constant = pointer;
    auto from_constant = LocalPtr<Path>(constant);
    REQUIRE(from_constant->m_value == "copy");
    auto moved = LocalPtr<Path>(std::move(pointer));
    REQUIRE(moved->m_value == "move");
  }

  TEST_CASE("constructing_from_an_incompatible_type_is_rejected") {
    REQUIRE(!(std::is_constructible_v<LocalPtr<int>, const char*>));
    REQUIRE(!(std::is_constructible_v<LocalPtr<int>, Path>));
    REQUIRE((std::is_constructible_v<LocalPtr<int>, int>));
    REQUIRE((std::is_constructible_v<LocalPtr<std::string>, const char*>));
  }

  TEST_CASE("assigning_a_value") {
    auto pointer = LocalPtr<Path>();
    const auto value = Path();
    pointer = value;
    REQUIRE(pointer->m_value == "copy assignment");
    pointer = Path();
    REQUIRE(pointer->m_value == "move assignment");
    auto other = LocalPtr<Path>();
    pointer = other;
    REQUIRE(pointer->m_value == "copy assignment");
    pointer = std::move(other);
    REQUIRE(pointer->m_value == "move assignment");
  }

  TEST_CASE("assigning_a_value_does_not_reconstruct_it") {
    auto pointer = LocalPtr<Allocation>();
    auto address = pointer->m_value.get();
    auto count = Allocation::count;
    auto value = Allocation();
    *value.m_value = 7;
    pointer = value;
    REQUIRE(Allocation::count == count + 1);
    REQUIRE(pointer->m_value.get() == address);
    REQUIRE(*pointer->m_value == 7);
  }

  TEST_CASE("assigning_an_incompatible_type_is_rejected") {
    REQUIRE(!(std::is_assignable_v<LocalPtr<int>&, const char*>));
    REQUIRE((std::is_assignable_v<LocalPtr<int>&, int>));
    REQUIRE((std::is_assignable_v<LocalPtr<int>&, LocalPtr<int>>));
  }

  TEST_CASE("dereferencing_propagates_constness") {
    auto pointer = LocalPtr<int>(1);
    REQUIRE((std::is_same_v<decltype(*pointer), int&>));
    REQUIRE((std::is_same_v<decltype(pointer.operator ->()), int*>));
    const auto& constant = pointer;
    REQUIRE((std::is_same_v<decltype(*constant), const int&>));
    REQUIRE((std::is_same_v<decltype(constant.operator ->()), const int*>));
    REQUIRE((std::is_same_v<decltype(*LocalPtr<int>(1)), int&>));
    *pointer = 2;
    REQUIRE(*constant == 2);
    REQUIRE(*pointer.operator ->() == 2);
  }

  TEST_CASE("the_wrapped_value_is_reached_only_by_dereferencing") {
    REQUIRE(!(std::is_convertible_v<LocalPtr<int>, int>));
    REQUIRE(!(std::is_convertible_v<LocalPtr<int>&, int&>));
    REQUIRE(!(std::is_convertible_v<const LocalPtr<int>&, const int&>));
  }

  TEST_CASE("operations_are_noexcept_when_the_value_is") {
    REQUIRE((std::is_nothrow_constructible_v<LocalPtr<Throws>, int>));
    REQUIRE(!(std::is_nothrow_constructible_v<LocalPtr<Throws>>));
    REQUIRE((std::is_nothrow_assignable_v<LocalPtr<Throws>&, double>));
    REQUIRE(!(std::is_nothrow_assignable_v<LocalPtr<Throws>&, int>));
    REQUIRE(noexcept(*std::declval<LocalPtr<Throws>&>()));
    REQUIRE(noexcept(std::declval<LocalPtr<Throws>&>().operator ->()));
  }

  TEST_CASE("try_ptr_t_wraps_only_non_pointers") {
    REQUIRE((std::is_same_v<try_ptr_t<int>, LocalPtr<int>>));
    REQUIRE((std::is_same_v<try_ptr_t<Path>, LocalPtr<Path>>));
    REQUIRE((std::is_same_v<try_ptr_t<int*>, int*>));
    REQUIRE((std::is_same_v<try_ptr_t<std::shared_ptr<int>>,
      std::shared_ptr<int>>));
    REQUIRE((std::is_same_v<try_ptr_t<LocalPtr<int>>, LocalPtr<int>>));
    REQUIRE(Dereferenceable<int*>);
    REQUIRE(!Dereferenceable<int>);
  }

  TEST_CASE("deducing_the_wrapped_type") {
    auto value = 1;
    REQUIRE((std::is_same_v<decltype(LocalPtr(value)), LocalPtr<int>>));
    REQUIRE((std::is_same_v<decltype(LocalPtr(std::string("a"))),
      LocalPtr<std::string>>));
    auto pointer = LocalPtr(value);
    REQUIRE((std::is_same_v<decltype(LocalPtr(pointer)), LocalPtr<int>>));
  }
}
