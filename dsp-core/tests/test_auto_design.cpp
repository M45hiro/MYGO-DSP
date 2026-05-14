#include <gtest/gtest.h>
#include "AutoFilterDesigner.h"

using namespace mygo_dsp;

TEST(AutoDesignTest, LowPassDesign) {
    FilterSpec spec;
    spec.type = FilterSpec::LowPass;
    spec.fs = 44100.0;
    spec.fpass = 500.0;
    spec.fstop = 1000.0;
    spec.Ap = 1.0;
    spec.As = 40.0;

    auto result = AutoFilterDesigner::autoDesign(spec);
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.order, 0);
    EXPECT_LE(result.order, 20);
}

TEST(AutoDesignTest, HighPassDesign) {
    FilterSpec spec;
    spec.type = FilterSpec::HighPass;
    spec.fs = 44100.0;
    spec.fpass = 5000.0;
    spec.fstop = 3000.0;
    spec.Ap = 1.0;
    spec.As = 40.0;

    auto result = AutoFilterDesigner::autoDesign(spec);
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.order, 0);
}

TEST(AutoDesignTest, BandPassDesign) {
    FilterSpec spec;
    spec.type = FilterSpec::BandPass;
    spec.fs = 44100.0;
    spec.fpass = 500.0;
    spec.fstop = 300.0;
    spec.fpass2 = 2000.0;
    spec.fstop2 = 3000.0;
    spec.Ap = 1.0;
    spec.As = 40.0;

    auto result = AutoFilterDesigner::autoDesign(spec);
    EXPECT_TRUE(result.success);
}

TEST(AutoDesignTest, BandStopDesign) {
    FilterSpec spec;
    spec.type = FilterSpec::BandStop;
    spec.fs = 44100.0;
    spec.fpass = 300.0;
    spec.fstop = 500.0;
    spec.fpass2 = 3000.0;
    spec.fstop2 = 2000.0;
    spec.Ap = 1.0;
    spec.As = 40.0;

    auto result = AutoFilterDesigner::autoDesign(spec);
    EXPECT_TRUE(result.success || result.useFIR);
}

TEST(AutoDesignTest, FallbackToFIR) {
    FilterSpec spec;
    spec.type = FilterSpec::LowPass;
    spec.fs = 44100.0;
    spec.fpass = 1000.0;
    spec.fstop = 1010.0;
    spec.Ap = 0.1;
    spec.As = 80.0;

    auto result = AutoFilterDesigner::autoDesign(spec, 20);
    EXPECT_TRUE(result.success || result.useFIR);
}

TEST(AutoDesignTest, InvalidSpec) {
    FilterSpec spec;
    spec.type = FilterSpec::LowPass;
    spec.fs = 44100.0;
    spec.fpass = 2000.0;
    spec.fstop = 1000.0;
    spec.Ap = 1.0;
    spec.As = 40.0;

    auto result = AutoFilterDesigner::autoDesign(spec);
    EXPECT_FALSE(result.success);
}

TEST(AutoDesignTest, OrderEstimateButterworth) {
    double wp = 0.1, ws = 0.2;
    int N = AutoFilterDesigner::estimateOrderButterworth(wp, ws, 1.0, 40.0);
    EXPECT_GT(N, 0);
    EXPECT_LT(N, 20);
}

TEST(AutoDesignTest, OrderEstimateElliptic) {
    double wp = 0.1, ws = 0.2;
    int N = AutoFilterDesigner::estimateOrderElliptic(wp, ws, 1.0, 40.0);
    EXPECT_GT(N, 0);
    EXPECT_LT(N, 20);
}
