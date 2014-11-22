/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#include "gtest/gtest.h"

#include <pdal/StageFactory.hpp>
#include <ReprojectionFilter.hpp>
#include <StatsFilter.hpp>

#include "Support.hpp"

using namespace pdal;

TEST(StatsFilterTest, StatsFilterTest_test1)
{
    BOX3D bounds(1.0, 2.0, 3.0, 101.0, 102.0, 103.0);
    Options ops;
    ops.add("bounds", bounds);
    ops.add("count", 1000);
    ops.add("mode", "constant");

    StageFactory f;
    StageFactory::ReaderCreator* rc = f.getReaderCreator("readers.faux");
    if (rc)
    {
        EXPECT_TRUE(rc);

        Stage* reader = rc();
        reader->setOptions(ops);

        StatsFilter filter;
        filter.setInput(reader);
        EXPECT_EQ(filter.getName(), "filters.stats");
        EXPECT_EQ(filter.getDescription(), "Statistics Filter");

        PointContext ctx;
        filter.prepare(ctx);
        filter.execute(ctx);

        const stats::Summary& statsX = filter.getStats(Dimension::Id::X);
        const stats::Summary& statsY = filter.getStats(Dimension::Id::Y);
        const stats::Summary& statsZ = filter.getStats(Dimension::Id::Z);

        EXPECT_EQ(statsX.count(), 1000u);
        EXPECT_EQ(statsY.count(), 1000u);
        EXPECT_EQ(statsZ.count(), 1000u);

        EXPECT_FLOAT_EQ(statsX.minimum(), 1.0);
        EXPECT_FLOAT_EQ(statsY.minimum(), 2.0);
        EXPECT_FLOAT_EQ(statsZ.minimum(), 3.0);

        EXPECT_FLOAT_EQ(statsX.maximum(), 1.0);
        EXPECT_FLOAT_EQ(statsY.maximum(), 2.0);
        EXPECT_FLOAT_EQ(statsZ.maximum(), 3.0);

        EXPECT_FLOAT_EQ(statsX.average(), 1.0);
        EXPECT_FLOAT_EQ(statsY.average(), 2.0);
        EXPECT_FLOAT_EQ(statsZ.average(), 3.0);
    }
}


TEST(StatsFilterTest, test_multiple_dims_same_name)
{
    const char* epsg4326_wkt = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4326\"]]";
    const SpatialReference out_ref(epsg4326_wkt);

    Options options;

    Option debug("debug", true, "");
    Option verbose("verbose", 5, "");
    // options.add(debug);
    // options.add(verbose);
    Option out_srs("out_srs",out_ref.getWKT(), "Output SRS to reproject to");
    Option spatialreference("spatialreference","EPSG:2993",
                            "Output SRS to reproject to");

    Option filename("filename", Support::datapath("las/1.2-with-color.las"), "");
    Option ignore("ignore_old_dimensions", false, "");
    options.add(out_srs);
    options.add(spatialreference);
    options.add(filename);
    options.add(ignore);

    StageFactory f;
    StageFactory::ReaderCreator* rc = f.getReaderCreator("readers.las");

    if (rc)
    {
        EXPECT_TRUE(rc);

        Stage* reader = rc();
        reader->setOptions(options);
        ReprojectionFilter reprojectionFilter;
        reprojectionFilter.setOptions(options);
        reprojectionFilter.setInput(reader);

        StatsFilter filter;
        filter.setOptions(options);
        filter.setInput(&reprojectionFilter);

        PointContext ctx;
        filter.prepare(ctx);
        filter.execute(ctx);

        const stats::Summary& statsX = filter.getStats(Dimension::Id::X);
        const stats::Summary& statsY = filter.getStats(Dimension::Id::Y);
        const stats::Summary& statsZ = filter.getStats(Dimension::Id::Z);

        EXPECT_EQ(statsX.count(), 1065u);
        EXPECT_EQ(statsY.count(), 1065u);
        EXPECT_EQ(statsZ.count(), 1065u);
    }
}


TEST(StatsFilterTest, test_specified_stats)
{
    const char* epsg4326_wkt = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4326\"]]";
    const SpatialReference out_ref(epsg4326_wkt);

    Options options;

    Option debug("debug", true);
    Option verbose("verbose", 5);
    // options.add(debug);
    // options.add(verbose);
    Option out_srs("out_srs",out_ref.getWKT(), "Output SRS to reproject to");
    Option spatialreference("spatialreference", "EPSG:2993",
                            "Output SRS to reproject to");

    Option filename("filename", Support::datapath("las/1.2-with-color.las"), "");
    Option ignore("ignore_old_dimensions", false, "");
    options.add(out_srs);
    options.add(spatialreference);
    options.add(filename);
    options.add(ignore);

    StageFactory f;
    StageFactory::ReaderCreator* rc = f.getReaderCreator("readers.las");

    if (rc)
    {
        EXPECT_TRUE(rc);

        Stage* reader = rc();
        reader->setOptions(options);

        Options stats1ops;
        stats1ops.add("dimensions", "Y");

        StatsFilter filter1;
        filter1.setOptions(stats1ops);
        filter1.setInput(reader);

        ReprojectionFilter reprojectionFilter;
        reprojectionFilter.setOptions(options);
        reprojectionFilter.setInput(&filter1);

        Options stats2ops;
        stats2ops.add("dimensions", "X Z");

        StatsFilter filter2;
        filter2.setOptions(stats2ops);
        filter2.setInput(&reprojectionFilter);

        PointContext ctx;
        filter2.prepare(ctx);
        filter2.execute(ctx);

        const stats::Summary& statsX = filter2.getStats(Dimension::Id::X);
        const stats::Summary& statsY = filter1.getStats(Dimension::Id::Y);
        const stats::Summary& statsZ = filter2.getStats(Dimension::Id::Z);

        EXPECT_EQ(statsX.count(), 1065u);
        EXPECT_EQ(statsY.count(), 1065u);
        EXPECT_EQ(statsZ.count(), 1065u);

        EXPECT_FLOAT_EQ(statsX.minimum(), -117.2686466233);
        EXPECT_FLOAT_EQ(statsY.minimum(), 848899.700);
    }
}


TEST(StatsFilterTest, test_pointbuffer_stats)
{

    const char* epsg4326_wkt = "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4326\"]]";
    const SpatialReference out_ref(epsg4326_wkt);

    Options options;

    Option dimensions("dimensions", "X,readers.las.Y Z filters.inplacereprojection.X, Classification", "");
    Option exact_dimensions("exact_dimensions", "Classification, X", "");

    Option debug("debug", true, "");
    Option verbose("verbose", 5, "");
    // options.add(debug);
    // options.add(verbose);
    Option out_srs("out_srs",out_ref.getWKT(), "Output SRS to reproject to");
    Option spatialreference("spatialreference","EPSG:2993",
                            "Output SRS to reproject to");
    Option x_dim("x_dim", "X", "Dimension name to use for 'X' data");
    Option y_dim("y_dim", "Y", "Dimension name to use for 'Y' data");
    Option z_dim("z_dim", "Z", "Dimension name to use for 'Z' data");
    Option x_scale("scale_x", 0.0000001f, "Scale for output X data in the "
                   "case when 'X' dimension data are to be scaled.  Defaults to '1.0'.  "
                   "If not set, the Dimensions's scale will be used");
    Option y_scale("scale_y", 0.0000001f, "Scale for output Y data in the "
                   "case when 'Y' dimension data are to be scaled.  Defaults to '1.0'.  "
                   "If not set, the Dimensions's scale will be used");
    Option filename("filename", Support::datapath("las/1.2-with-color.las"));
    Option ignore("ignore_old_dimensions", false);
    options.add(out_srs);
    options.add(x_dim);
    options.add(y_dim);
    options.add(z_dim);
    options.add(x_scale);
    options.add(y_scale);
    options.add(spatialreference);
    options.add(filename);
    options.add(ignore);
    options.add(dimensions);
    options.add(exact_dimensions);

    StageFactory f;
    StageFactory::ReaderCreator* rc = f.getReaderCreator("readers.las");

    if (rc)
    {
        EXPECT_TRUE(rc);

        Stage* reader = rc();
        reader->setOptions(options);

        ReprojectionFilter reprojectionFilter;
        reprojectionFilter.setOptions(options);
        reprojectionFilter.setInput(reader);

        Options statsOptions = options;
        options.add("num_points", 1000);

        StatsFilter filter;
        filter.setOptions(options);
        filter.setInput(&reprojectionFilter);

        PointContext ctx;
        filter.prepare(ctx);
        filter.execute(ctx);

        MetadataNode m = ctx.metadata();
        m = m.findChild("filters.stats:statistic:counts:count-1:count");
        EXPECT_EQ(m.value(), "737");
    }
}
