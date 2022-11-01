#include "chalk/combinatorics/dyck_path.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace chalk {

using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

TEST(DyckPath, DefaultConstruction) {
  DyckPath p;
  EXPECT_THAT(p.size(), 0);
  EXPECT_THAT(p, SizeIs(0));
  EXPECT_THAT(p, ElementsAre());
  EXPECT_THAT(p, IsEmpty());
}

TEST(DyckPath, InitializerListConstruction) {
  DyckPath p{DyckPath::Step::Up, DyckPath::Step::Down};
  EXPECT_THAT(p.size(), 2);
  EXPECT_THAT(p, SizeIs(2));
  EXPECT_THAT(p, ElementsAre(DyckPath::Step::Up, DyckPath::Step::Down));
  EXPECT_EQ(p[0], DyckPath::Step::Up);
  EXPECT_EQ(p[1], DyckPath::Step::Down);
}

TEST(DyckPath, Area) {
  EXPECT_THAT(Area(DyckPath{}), 0);
  EXPECT_THAT(Area(DyckPath{DyckPath::Step::Up, DyckPath::Step::Down}), 0);
  EXPECT_THAT(Area(DyckPath{DyckPath::Step::Up, DyckPath::Step::Down,
                            DyckPath::Step::Up, DyckPath::Step::Down}),
              0);
  EXPECT_THAT(Area(DyckPath{DyckPath::Step::Up, DyckPath::Step::Down,
                            DyckPath::Step::Up, DyckPath::Step::Down,
                            DyckPath::Step::Up, DyckPath::Step::Down}),
              0);
  EXPECT_THAT(Area(DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                            DyckPath::Step::Down, DyckPath::Step::Down}),
              1);
  EXPECT_THAT(Area(DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                            DyckPath::Step::Up, DyckPath::Step::Down,
                            DyckPath::Step::Down, DyckPath::Step::Down}),
              3);
  EXPECT_THAT(Area(DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                            DyckPath::Step::Up, DyckPath::Step::Up,
                            DyckPath::Step::Down, DyckPath::Step::Down,
                            DyckPath::Step::Down, DyckPath::Step::Down}),
              6);

  //   /\/\   |
  //  /\/\/\  |
  // /\/\/\/\ |
  EXPECT_THAT(Area(DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                            DyckPath::Step::Up, DyckPath::Step::Down,
                            DyckPath::Step::Up, DyckPath::Step::Down,
                            DyckPath::Step::Down, DyckPath::Step::Down}),
              5);

  //   /\     |
  //  /\/\/\  |
  // /\/\/\/\ |
  EXPECT_THAT(Area(DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                            DyckPath::Step::Up, DyckPath::Step::Down,
                            DyckPath::Step::Down, DyckPath::Step::Up,
                            DyckPath::Step::Down, DyckPath::Step::Down}),
              4);
}

TEST(DyckPath, Bounce) {
  EXPECT_THAT(Bounce(DyckPath{}), 0);
  EXPECT_THAT(Bounce(DyckPath{DyckPath::Step::Up, DyckPath::Step::Down}), 0);
  EXPECT_THAT(Bounce(DyckPath{DyckPath::Step::Up, DyckPath::Step::Down,
                              DyckPath::Step::Up, DyckPath::Step::Down}),
              1);
  EXPECT_THAT(Bounce(DyckPath{DyckPath::Step::Up, DyckPath::Step::Down,
                              DyckPath::Step::Up, DyckPath::Step::Down,
                              DyckPath::Step::Up, DyckPath::Step::Down}),
              3);
  EXPECT_THAT(Bounce(DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                              DyckPath::Step::Down, DyckPath::Step::Down}),
              0);
  EXPECT_THAT(Bounce(DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                              DyckPath::Step::Up, DyckPath::Step::Down,
                              DyckPath::Step::Down, DyckPath::Step::Down}),
              0);
  EXPECT_THAT(Bounce(DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                              DyckPath::Step::Up, DyckPath::Step::Up,
                              DyckPath::Step::Down, DyckPath::Step::Down,
                              DyckPath::Step::Down, DyckPath::Step::Down}),
              0);

  //   /\/\   |
  //  /\/\/\  |
  // /\/\/\/\ |
  EXPECT_THAT(Bounce(DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                              DyckPath::Step::Up, DyckPath::Step::Down,
                              DyckPath::Step::Up, DyckPath::Step::Down,
                              DyckPath::Step::Down, DyckPath::Step::Down}),
              1);

  //   /\     |
  //  /\/\/\  |
  // /\/\/\/\ |
  EXPECT_THAT(Bounce(DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                              DyckPath::Step::Up, DyckPath::Step::Down,
                              DyckPath::Step::Down, DyckPath::Step::Up,
                              DyckPath::Step::Down, DyckPath::Step::Down}),
              1);

  //     /\   |
  //  /\/\/\  |
  // /\/\/\/\ |
  EXPECT_THAT(Bounce(DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                              DyckPath::Step::Down, DyckPath::Step::Up,
                              DyckPath::Step::Up, DyckPath::Step::Down,
                              DyckPath::Step::Down, DyckPath::Step::Down}),
              2);
}

TEST(DyckPath, Lift) {
  EXPECT_EQ(DyckPath::Lift(DyckPath{DyckPath::Step::Up, DyckPath::Step::Down}),
            (DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                      DyckPath::Step::Down, DyckPath::Step::Down}));
  EXPECT_EQ(DyckPath::Lift(DyckPath{DyckPath::Step::Up, DyckPath::Step::Down,
                                    DyckPath::Step::Up, DyckPath::Step::Down}),
            (DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                      DyckPath::Step::Down, DyckPath::Step::Up,
                      DyckPath::Step::Down, DyckPath::Step::Down}));
}

TEST(DyckPath, Peak) {
  EXPECT_EQ(DyckPath::Peak(0), DyckPath{});
  EXPECT_EQ(DyckPath::Peak(1), (DyckPath{
                                   DyckPath::Step::Up,
                                   DyckPath::Step::Down,
                               }));
  EXPECT_EQ(DyckPath::Peak(2), (DyckPath{
                                   DyckPath::Step::Up,
                                   DyckPath::Step::Up,
                                   DyckPath::Step::Down,
                                   DyckPath::Step::Down,
                               }));
  EXPECT_EQ(DyckPath::Peak(3), (DyckPath{
                                   DyckPath::Step::Up,
                                   DyckPath::Step::Up,
                                   DyckPath::Step::Up,
                                   DyckPath::Step::Down,
                                   DyckPath::Step::Down,
                                   DyckPath::Step::Down,
                               }));
  EXPECT_EQ(DyckPath::Peak(4), (DyckPath{
                                   DyckPath::Step::Up,
                                   DyckPath::Step::Up,
                                   DyckPath::Step::Up,
                                   DyckPath::Step::Up,
                                   DyckPath::Step::Down,
                                   DyckPath::Step::Down,
                                   DyckPath::Step::Down,
                                   DyckPath::Step::Down,
                               }));
}

TEST(DyckPath, Minimal) {
  EXPECT_EQ(DyckPath::Minimal(0), DyckPath{});
  EXPECT_EQ(DyckPath::Minimal(1), (DyckPath{
                                      DyckPath::Step::Up,
                                      DyckPath::Step::Down,
                                  }));
  EXPECT_EQ(DyckPath::Minimal(2), (DyckPath{
                                      DyckPath::Step::Up,
                                      DyckPath::Step::Down,
                                      DyckPath::Step::Up,
                                      DyckPath::Step::Down,
                                  }));
  EXPECT_EQ(DyckPath::Minimal(3), (DyckPath{
                                      DyckPath::Step::Up,
                                      DyckPath::Step::Down,
                                      DyckPath::Step::Up,
                                      DyckPath::Step::Down,
                                      DyckPath::Step::Up,
                                      DyckPath::Step::Down,
                                  }));
  EXPECT_EQ(DyckPath::Minimal(4), (DyckPath{
                                      DyckPath::Step::Up,
                                      DyckPath::Step::Down,
                                      DyckPath::Step::Up,
                                      DyckPath::Step::Down,
                                      DyckPath::Step::Up,
                                      DyckPath::Step::Down,
                                      DyckPath::Step::Up,
                                      DyckPath::Step::Down,
                                  }));
}

TEST(DyckPath, Concatenate) {
  EXPECT_EQ(DyckPath::Concatenate(
                DyckPath{}, DyckPath{DyckPath::Step::Up, DyckPath::Step::Down}),
            (DyckPath{DyckPath::Step::Up, DyckPath::Step::Down}));
  EXPECT_EQ(DyckPath::Concatenate(
                DyckPath{DyckPath::Step::Up, DyckPath::Step::Down}, DyckPath{}),
            (DyckPath{DyckPath::Step::Up, DyckPath::Step::Down}));
  EXPECT_EQ(
      DyckPath::Concatenate(DyckPath{DyckPath::Step::Up, DyckPath::Step::Down},
                            DyckPath{DyckPath::Step::Up, DyckPath::Step::Down}),
      (DyckPath{DyckPath::Step::Up, DyckPath::Step::Down, DyckPath::Step::Up,
                DyckPath::Step::Down}));

  EXPECT_EQ(DyckPath::Concatenate(
                DyckPath{}, DyckPath{DyckPath::Step::Up, DyckPath::Step::Down},
                DyckPath{}),
            (DyckPath{DyckPath::Step::Up, DyckPath::Step::Down}));
  EXPECT_EQ(DyckPath::Concatenate(DyckPath::Minimal(1), DyckPath::Minimal(2),
                                  DyckPath::Minimal(3), DyckPath::Minimal(4)),
            DyckPath::Minimal(10));
}

TEST(DyckPath, All) {
  EXPECT_THAT(DyckPath::All(0), UnorderedElementsAre(DyckPath{}));
  EXPECT_THAT(DyckPath::All(1), UnorderedElementsAre(DyckPath{
                                    DyckPath::Step::Up, DyckPath::Step::Down}));
  EXPECT_THAT(
      DyckPath::All(2),
      UnorderedElementsAre(DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                                    DyckPath::Step::Down, DyckPath::Step::Down},
                           DyckPath{DyckPath::Step::Up, DyckPath::Step::Down,
                                    DyckPath::Step::Up, DyckPath::Step::Down}));
  EXPECT_THAT(
      DyckPath::All(3),
      UnorderedElementsAre(DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                                    DyckPath::Step::Up, DyckPath::Step::Down,
                                    DyckPath::Step::Down, DyckPath::Step::Down},
                           DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                                    DyckPath::Step::Down, DyckPath::Step::Down,
                                    DyckPath::Step::Up, DyckPath::Step::Down},
                           DyckPath{DyckPath::Step::Up, DyckPath::Step::Up,
                                    DyckPath::Step::Down, DyckPath::Step::Up,
                                    DyckPath::Step::Down, DyckPath::Step::Down},
                           DyckPath{DyckPath::Step::Up, DyckPath::Step::Down,
                                    DyckPath::Step::Up, DyckPath::Step::Up,
                                    DyckPath::Step::Down, DyckPath::Step::Down},
                           DyckPath{DyckPath::Step::Up, DyckPath::Step::Down,
                                    DyckPath::Step::Up, DyckPath::Step::Down,
                                    DyckPath::Step::Up, DyckPath::Step::Down}));
}

}  // namespace chalk
