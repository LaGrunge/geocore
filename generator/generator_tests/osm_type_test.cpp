#include "testing/testing.hpp"

#include "generator/generator_tests/types_helper.hpp"
#include "generator/generator_tests_support/test_with_classificator.hpp"
#include "generator/osm_element.hpp"
#include "generator/osm2type.hpp"
#include "generator/tag_admixer.hpp"

#include "indexer/feature_data.hpp"
#include "indexer/classificator.hpp"
#include "indexer/classificator_loader.hpp"

#include "platform/platform.hpp"

#include <iostream>
#include <string>
#include <vector>

using namespace generator::tests_support;
using namespace tests;

namespace
{
  void DumpTypes(std::vector<uint32_t> const & v)
  {
    Classificator const & c = classif();
    for (size_t i = 0; i < v.size(); ++i)
      std::cout << c.GetFullObjectName(v[i]) << std::endl;
  }

  void DumpParsedTypes(char const * arr[][2], size_t count)
  {
    OsmElement e;
    FillXmlElement(arr, count, &e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    DumpTypes(params.m_types);
  }

  void TestSurfaceTypes(std::string const & surface, std::string const & smoothness,
                        std::string const & grade, char const * value)
  {
    OsmElement e;
    e.AddTag("highway", "unclassified");
    e.AddTag("surface", surface);
    e.AddTag("smoothness", smoothness);
    e.AddTag("surface:grade", grade);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_types.size(), 2, (params));
    TEST(params.IsTypeExist(GetType({"highway", "unclassified"})), ());
    std::string psurface;
    for (auto type : params.m_types)
    {
      std::string const rtype = classif().GetReadableObjectName(type);
      if (rtype.substr(0, 9) == "psurface-")
        psurface = rtype.substr(9);
    }
    TEST(params.IsTypeExist(GetType({"psurface", value})), ("Surface:", surface, "Smoothness:", smoothness, "Grade:", grade, "Expected:", value, "Got:", psurface));
  }

FeatureParams GetFeatureParams(char const * arr[][2], size_t count)
  {
    OsmElement e;
    FillXmlElement(arr, count, &e);
    FeatureParams params;

    ftype::GetNameAndType(&e, params);
    return params;
  }
}  // namespace

UNIT_CLASS_TEST(TestWithClassificator, OsmType_SkipDummy)
{
  char const * arr[][2] = {
    { "abutters", "residential" },
    { "highway", "primary" },
    { "osmarender:renderRef", "no" },
    { "ref", "E51" }
  };

  FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

  TEST_EQUAL(params.m_types.size(), 1, (params));
  TEST_EQUAL(params.m_types[0], GetType(arr[1]), ());
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Check)
{
  char const * arr1[][2] = {
    { "highway", "primary" },
    { "motorroad", "yes" },
    { "name", "Каширское шоссе" },
    { "oneway", "yes" }
  };

  char const * arr2[][2] = {
    { "highway", "primary" },
    { "name", "Каширское шоссе" },
    { "oneway", "-1" },
    { "motorroad", "yes" }
  };

  char const * arr3[][2] = {
    { "admin_level", "4" },
    { "border_type", "state" },
    { "boundary", "administrative" }
  };

  char const * arr4[][2] = {
    { "border_type", "state" },
    { "admin_level", "4" },
    { "boundary", "administrative" }
  };

  DumpParsedTypes(arr1, ARRAY_SIZE(arr1));
  DumpParsedTypes(arr2, ARRAY_SIZE(arr2));
  DumpParsedTypes(arr3, ARRAY_SIZE(arr3));
  DumpParsedTypes(arr4, ARRAY_SIZE(arr4));
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Combined)
{
  char const * arr[][2] = {
    { "addr:housenumber", "84" },
    { "addr:postcode", "220100" },
    { "addr:street", "ул. Максима Богдановича" },
    { "amenity", "school" },
    { "building", "yes" },
    { "name", "Гимназия 15" }
  };

  FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

  TEST_EQUAL(params.m_types.size(), 2, (params));
  TEST(params.IsTypeExist(GetType(arr[3])), ());
  TEST(params.IsTypeExist(GetType({"building"})), ());

  std::string s;
  params.name.GetString(0, s);
  TEST_EQUAL(s, arr[5][1], ());

  TEST_EQUAL(params.house.Get(), "84", ());
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Address)
{
  char const * arr[][2] = {
    { "addr:conscriptionnumber", "223" },
    { "addr:housenumber", "223/5" },
    { "addr:postcode", "11000" },
    { "addr:street", "Řetězová" },
    { "addr:streetnumber", "5" },
    { "source:addr", "uir_adr" },
    { "uir_adr:ADRESA_KOD", "21717036" }
  };

  FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

  TEST_EQUAL(params.m_types.size(), 1, (params));
  TEST(params.IsTypeExist(GetType({"building", "address"})), ());

  TEST_EQUAL(params.house.Get(), "223/5", ());
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_PlaceState)
{
  char const * arr[][2] = {
    { "alt_name:vi", "California" },
    { "is_in", "USA" },
    { "is_in:continent", "North America" },
    { "is_in:country", "USA" },
    { "is_in:country_code", "us" },
    { "name", "California" },
    { "place", "state" },
    { "population", "37253956" },
    { "ref", "CA" }
  };

  FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

  TEST_EQUAL(params.m_types.size(), 1, (params));
  TEST(params.IsTypeExist(GetType({"place", "state", "USA"})), ());

  std::string s;
  TEST(params.name.GetString(0, s), ());
  TEST_EQUAL(s, "California", ());
  TEST_GREATER(params.rank, 1, ());
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_AlabamaRiver)
{
  char const * arr1[][2] = {
    { "NHD:FCode", "55800" },
    { "NHD:FType", "558" },
    { "NHD:RESOLUTION", "2" },
    { "NHD:way_id", "139286586;139286577;139286596;139286565;139286574;139286508;139286600;139286591;139286507;139286505;139286611;139286602;139286594;139286604;139286615;139286616;139286608;139286514;139286511;139286564;139286576;139286521;139286554" },
    { "attribution", "NHD" },
    { "boat", "yes" },
    { "deep_draft", "no" },
    { "gnis:feature_id", "00517033" },
    { "name", "Tennessee River" },
    { "ship", "yes" },
    { "source", "NHD_import_v0.4_20100913205417" },
    { "source:deep_draft", "National Transportation Atlas Database 2011" },
    { "waterway", "river" }
  };

  char const * arr2[][2] = {
    { "destination", "Ohio River" },
    { "name", "Tennessee River" },
    { "type", "waterway" },
    { "waterway", "river" }
  };

  char const * arr3[][2] = {
    { "name", "Tennessee River" },
    { "network", "inland waterways" },
    { "route", "boat" },
    { "ship", "yes" },
    { "type", "route" }
  };

  OsmElement e;
  FillXmlElement(arr1, ARRAY_SIZE(arr1), &e);
  FillXmlElement(arr2, ARRAY_SIZE(arr2), &e);
  FillXmlElement(arr3, ARRAY_SIZE(arr3), &e);

  FeatureParams params;
  ftype::GetNameAndType(&e, params);

  TEST_EQUAL(params.m_types.size(), 1, (params));
  TEST(params.IsTypeExist(GetType({"waterway", "river"})), ());
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Synonyms)
{
  // Smoke test.
  {
    char const * arr[][2] = {
      { "building", "yes" },
      { "shop", "yes" },
      { "atm", "yes" },
      { "toilets", "yes" },
      { "restaurant", "yes" },
      { "hotel", "yes" },
    };

    OsmElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    TagReplacer tagReplacer(GetPlatform().ResourcesDir() + REPLACED_TAGS_FILE);
    tagReplacer(e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    char const * arrT1[] = { "building" };
    char const * arrT2[] = { "amenity", "atm" };
    char const * arrT3[] = { "shop" };
    char const * arrT4[] = { "amenity", "restaurant" };
    char const * arrT5[] = { "tourism", "hotel" };
    char const * arrT6[] = { "amenity", "toilets" };
    TEST_EQUAL(params.m_types.size(), 6, (params));

    TEST(params.IsTypeExist(GetType(arrT1)), ());
    TEST(params.IsTypeExist(GetType(arrT2)), ());
    TEST(params.IsTypeExist(GetType(arrT3)), ());
    TEST(params.IsTypeExist(GetType(arrT4)), ());
    TEST(params.IsTypeExist(GetType(arrT5)), ());
    TEST(params.IsTypeExist(GetType(arrT6)), ());
  }

  // Duplicating test.
  {
    char const * arr[][2] = {
      { "amenity", "atm" },
      { "atm", "yes" }
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"amenity", "atm"})), ());
  }

  // "NO" tag test.
  {
    char const * arr[][2] = {
      { "building", "yes" },
      { "shop", "no" },
      { "atm", "no" }
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"building"})), ());
  }
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Capital)
{
  {
    char const * arr[][2] = {
      { "admin_level", "6" },
      { "capital", "yes" },
      { "place", "city" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"place", "city", "capital", "6"})), ());
  }

  {
    char const * arr[][2] = {
      { "admin_level", "6" },
      { "capital", "no" },
      { "place", "city" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"place", "city"})), ());
  }

  {
    char const * arr[][2] = {
      { "place", "city" },
      { "admin_level", "4" },
      { "boundary", "administrative" },
      { "capital", "2" },
      { "place", "city" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 2, (params));
    TEST(params.IsTypeExist(GetType({"place", "city", "capital", "2"})), ());
    TEST(params.IsTypeExist(GetType({"boundary", "administrative", "4"})), ());
  }
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Route)
{
  {
    char const * arr[][2] = {
      { "highway", "motorway" },
      { "ref", "I 95" }
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType(arr[0])), ());
    TEST_EQUAL(params.ref, arr[1][1], ());
  }
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Layer)
{
  {
    char const * arr[][2] = {
      { "highway", "motorway" },
      { "bridge", "yes" },
      { "layer", "2" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"highway", "motorway", "bridge"})), ());
    TEST_EQUAL(params.layer, 2, ());
  }

  {
    char const * arr[][2] = {
      { "highway", "trunk" },
      { "tunnel", "yes" },
      { "layer", "-1" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"highway", "trunk", "tunnel"})), ());
    TEST_EQUAL(params.layer, -1, ());
  }

  {
    char const * arr[][2] = {
      { "highway", "secondary" },
      { "bridge", "yes" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"highway", "secondary", "bridge"})), ());
    TEST_EQUAL(params.layer, 1, ());
  }

  {
    char const * arr[][2] = {
      { "highway", "primary" },
      { "tunnel", "yes" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"highway", "primary", "tunnel"})), ());
    TEST_EQUAL(params.layer, -1, ());
  }

  {
    char const * arr[][2] = {
      { "highway", "living_street" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType(arr[0])), ());
    TEST_EQUAL(params.layer, 0, ());
  }
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Amenity)
{
  {
    char const * arr[][2] = {
      { "amenity", "bbq" },
      { "fuel", "wood" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType(arr[0])), ());
  }
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Hwtag)
{
  char const * tags[][2] = {
      {"hwtag", "oneway"}, {"hwtag", "private"}, {"hwtag", "lit"}, {"hwtag", "nofoot"}, {"hwtag", "yesfoot"},
      {"hwtag", "yesbicycle"}, {"hwtag", "bidir_bicycle"}
  };

  {
    char const * arr[][2] = {
      { "railway", "rail" },
      { "access", "private" },
      { "oneway", "true" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType(arr[0])), ());
  }

  {
    char const * arr[][2] = {
        {"oneway", "-1"},
        {"highway", "primary"},
        {"access", "private"},
        {"lit", "no"},
        {"foot", "no"},
        {"bicycle", "yes"},
        {"oneway:bicycle", "no"},
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 6, (params));
    TEST(params.IsTypeExist(GetType(arr[1])), ());
    TEST(params.IsTypeExist(GetType(tags[0])), ());
    TEST(params.IsTypeExist(GetType(tags[1])), ());
    TEST(params.IsTypeExist(GetType(tags[3])), ());
    TEST(params.IsTypeExist(GetType(tags[5])), ());
    TEST(params.IsTypeExist(GetType(tags[6])), ());
  }

  {
    char const * arr[][2] = {
        {"foot", "yes"}, {"cycleway", "lane"}, {"highway", "primary"},
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 3, (params));
    TEST(params.IsTypeExist(GetType(arr[2])), ());
    TEST(params.IsTypeExist(GetType(tags[4])), ());
    TEST(params.IsTypeExist(GetType(tags[5])), ());
  }
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Surface)
{
  TestSurfaceTypes("asphalt", "", "", "paved_good");
  TestSurfaceTypes("asphalt", "bad", "", "paved_bad");
  TestSurfaceTypes("asphalt", "", "0", "paved_bad");
  TestSurfaceTypes("paved", "", "2", "paved_good");
  TestSurfaceTypes("", "excellent", "", "paved_good");
  TestSurfaceTypes("wood", "", "", "paved_bad");
  TestSurfaceTypes("wood", "good", "", "paved_good");
  TestSurfaceTypes("wood", "", "3", "paved_good");
  TestSurfaceTypes("unpaved", "", "", "unpaved_good");
  TestSurfaceTypes("mud", "", "", "unpaved_bad");
  TestSurfaceTypes("", "bad", "", "unpaved_good");
  TestSurfaceTypes("", "horrible", "", "unpaved_bad");
  TestSurfaceTypes("ground", "", "1", "unpaved_bad");
  TestSurfaceTypes("mud", "", "3", "unpaved_good");
  TestSurfaceTypes("unknown", "", "", "unpaved_good");
  TestSurfaceTypes("", "unknown", "", "unpaved_good");
}


UNIT_CLASS_TEST(TestWithClassificator, OsmType_Boundary)
{
  char const * arr[][2] = {
    { "admin_level", "4" },
    { "boundary", "administrative" },
    { "admin_level", "2" },
    { "boundary", "administrative" },
  };

  FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

  TEST_EQUAL(params.m_types.size(), 2, (params));
  TEST(params.IsTypeExist(GetType({"boundary", "administrative", "2"})), ());
  TEST(params.IsTypeExist(GetType({"boundary", "administrative", "4"})), ());
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Dibrugarh)
{
  char const * arr[][2] = {
    { "AND_a_c", "10001373" },
    { "addr:city", "Dibrugarh" },
    { "addr:housenumber", "hotel vishal" },
    { "addr:postcode", "786001" },
    { "addr:street", "Marwari Patty,Puja Ghat" },
    { "name", "Dibrugarh" },
    { "phone", "03732320016" },
    { "place", "city" },
    { "website", "http://www.hotelvishal.in" },
  };

  FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

  TEST_EQUAL(params.m_types.size(), 1, (params));
  TEST(params.IsTypeExist(GetType({"place", "city"})), (params));
  std::string name;
  TEST(params.name.GetString(StringUtf8Multilang::kDefaultCode, name), (params));
  TEST_EQUAL(name, "Dibrugarh", (params));
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Subway)
{
  {
    char const * arr[][2] = {
      { "network", "Московский метрополитен" },
      { "operator", "ГУП «Московский метрополитен»" },
      { "railway", "station" },
      { "station", "subway" },
      { "transport", "subway" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"railway", "station", "subway", "moscow"})), (params));
  }

  {
    char const * arr[][2] = {
      { "name", "14th Street-8th Avenue (A,C,E,L)" },
      { "network", "New York City Subway" },
      { "railway", "station" },
      { "wheelchair", "yes" },
      { "route", "subway" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 2, (params));
    TEST(params.IsTypeExist(GetType({"railway", "station", "subway", "newyork"})), (params));
  }

  {
    char const * arr[][2] = {
      { "name", "S Landsberger Allee" },
      { "phone", "030 29743333" },
      { "public_transport", "stop_position" },
      { "railway", "station" },
      { "network", "New York City Subway" },
      { "station", "light_rail" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"railway", "station", "light_rail"})), (params));
  }

  {
    char const * arr[][2] = {
      { "monorail", "yes" },
      { "name", "Улица Академика Королёва" },
      { "network", "Московский метрополитен" },
      { "operator", "ГУП «Московский метрополитен»" },
      { "public_transport", "stop_position" },
      { "railway", "station" },
      { "station", "monorail" },
      { "transport", "monorail" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"railway", "station", "monorail"})), (params));
  }

  {
    char const * arr[][2] = {
      { "line",	"Northern, Bakerloo" },
      { "name",	"Charing Cross" },
      { "network", "London Underground" },
      { "operator", "TfL" },
      { "railway", "station" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"railway", "station", "subway", "london"})), (params));
  }
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Hospital)
{
  {
    char const * arr[][2] = {
      { "building", "hospital" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"building"})), (params));
  }

  {
    char const * arr[][2] = {
      { "building", "yes" },
      { "amenity", "hospital" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 2, (params));
    TEST(params.IsTypeExist(GetType({"building"})), (params));
    TEST(params.IsTypeExist(GetType({"amenity", "hospital"})), (params));
  }
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Entrance)
{
  {
    char const * arr[][2] = {
      { "building", "entrance" },
      { "barrier", "entrance" },
    };

    OsmElement e;
    FillXmlElement(arr, ARRAY_SIZE(arr), &e);

    TagReplacer tagReplacer(GetPlatform().ResourcesDir() + REPLACED_TAGS_FILE);
    tagReplacer(e);

    FeatureParams params;
    ftype::GetNameAndType(&e, params);

    TEST_EQUAL(params.m_types.size(), 2, (params));
    TEST(params.IsTypeExist(GetType({"entrance"})), (params));
    TEST(params.IsTypeExist(GetType({"barrier", "entrance"})), (params));
  }
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Moscow)
{
  {
    char const * arr[][2] = {
      { "addr:country", "RU" },
      { "addr:region", "Москва" },
      { "admin_level", "2" },
      { "alt_name:vi", "Mạc Tư Khoa" },
      { "capital", "yes" },
      { "ele", "156" },
      { "int_name", "Moscow" },
      { "is_capital", "country" },
      { "ISO3166-2", "RU-MOW" },
      { "name", "Москва" },
      { "note", "эта точка должна быть здесь, в историческом центре Москвы" },
      { "official_status", "ru:город" },
      { "okato:user", "none" },
      { "place", "city" },
      { "population", "12108257" },
      { "population:date", "2014-01-01" },
      { "rank", "0" },
      { "wikipedia", "ru:Москва" },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 1, (params));
    TEST(params.IsTypeExist(GetType({"place", "city", "capital", "2"})), (params));
    TEST(170 <= params.rank && params.rank <= 180, (params));
    TEST(!params.name.IsEmpty(), (params));
  }
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Translations)
{
  char const * arr[][2] = {
    { "name", "Paris" },
    { "name:ru", "Париж" },
    { "name:en", "Paris" },
    { "name:en:pronunciation", "ˈpæɹ.ɪs" },
    { "name:fr:pronunciation", "paʁi" },
    { "place", "city" },
    { "population", "2243833" }
  };

  FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

  TEST_EQUAL(params.m_types.size(), 1, (params));
  TEST(params.IsTypeExist(GetType({"place", "city"})), ());

  std::string name;
  TEST(params.name.GetString(StringUtf8Multilang::kDefaultCode, name), (params));
  TEST_EQUAL(name, "Paris", (params));
  TEST(params.name.GetString(StringUtf8Multilang::kEnglishCode, name), (params));
  TEST_EQUAL(name, "Paris", (params));
  TEST(!params.name.GetString("fr", name), (params));
  TEST(params.name.GetString("ru", name), (params));
  TEST_EQUAL(name, "Париж", (params));
}

UNIT_CLASS_TEST(TestWithClassificator, OsmType_Cuisine)
{
  {
    char const * arr[][2] = {
      { "cuisine", "indian ; steak,coffee  shop " },
    };

    FeatureParams const params = GetFeatureParams(arr, ARRAY_SIZE(arr));

    TEST_EQUAL(params.m_types.size(), 3, (params));
    TEST(params.IsTypeExist(GetType({"cuisine", "indian"})), (params));
    TEST(params.IsTypeExist(GetType({"cuisine", "steak_house"})), (params));
    TEST(params.IsTypeExist(GetType({"cuisine", "coffee_shop"})), (params));
  }
}
