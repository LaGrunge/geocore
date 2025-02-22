project(indexer)

set(
  SRC
  altitude_loader.cpp
  altitude_loader.hpp
  borders.cpp
  borders.hpp
  brands_holder.cpp
  brands_holder.hpp
  caching_rank_table_loader.cpp
  caching_rank_table_loader.hpp
  categories_holder.cpp
  categories_holder.hpp
  categories_holder_loader.cpp
  categories_index.cpp
  categories_index.hpp
  cell_coverer.hpp
  cell_id.hpp
  cell_value_pair.hpp
  centers_table.cpp
  centers_table.hpp
  cities_boundaries_serdes.hpp
  city_boundary.hpp
  classificator.cpp
  classificator.hpp
  classificator_loader.cpp
  classificator_loader.hpp
  data_factory.cpp
  data_factory.hpp
  data_header.cpp
  data_header.hpp
  data_source.cpp
  data_source.hpp
  data_source_helpers.cpp
  data_source_helpers.hpp
  drawing_rule_def.cpp
  drawing_rule_def.hpp
  drawing_rules.cpp
  drawing_rules.hpp
  drules_include.hpp
  drules_selector.cpp
  drules_selector.hpp
  drules_selector_parser.cpp
  drules_selector_parser.hpp
  drules_struct.pb.cc
  drules_struct.pb.h
  dummy_rank_table.cpp
  dummy_rank_table.hpp
  editable_map_object.cpp
  editable_map_object.hpp
  fake_feature_ids.cpp
  fake_feature_ids.hpp
  feature.cpp
  feature.hpp
  feature_algo.cpp
  feature_algo.hpp
  feature_altitude.hpp
  feature_covering.cpp
  feature_covering.hpp
  feature_data.cpp
  feature_data.hpp
  feature_decl.cpp
  feature_decl.hpp
  feature_impl.cpp
  feature_impl.hpp
  feature_meta.cpp
  feature_meta.hpp
  feature_processor.hpp
  feature_source.cpp
  feature_source.hpp
  feature_to_osm.cpp
  feature_to_osm.hpp
  feature_utils.cpp
  feature_utils.hpp
  feature_visibility.cpp
  feature_visibility.hpp
  features_offsets_table.cpp
  features_offsets_table.hpp
  features_vector.cpp
  features_vector.hpp
  ftraits.hpp
  ftypes_matcher.cpp
  ftypes_matcher.hpp
  index_builder.cpp
  index_builder.hpp
  interval_index.hpp
  interval_index_builder.hpp
  locality_index.cpp
  locality_index.hpp
  locality_index_builder.cpp
  locality_index_builder.hpp
  locality_object.cpp
  locality_object.hpp
  map_object.cpp
  map_object.hpp
  map_style.cpp
  map_style.hpp
  map_style_reader.cpp
  map_style_reader.hpp
  mwm_set.cpp
  mwm_set.hpp
  postcodes_matcher.cpp   # it's in indexer due to editor which is in indexer and depends on postcodes_marcher
  postcodes_matcher.hpp    # it's in indexer due to editor which is in indexer and depends on postcodes_marcher
  rank_table.cpp
  rank_table.hpp
  road_shields_parser.cpp
  road_shields_parser.hpp
  scale_index.hpp
  scale_index_builder.hpp
  scales.cpp
  scales.hpp
  scales_patch.hpp
  search_delimiters.cpp     # it's in indexer because of CategoriesHolder dependency.
  search_delimiters.hpp       # it's in indexer because of CategoriesHolder dependency.
  search_string_utils.cpp   # it's in indexer because of CategoriesHolder dependency.
  search_string_utils.hpp     # it's in indexer because of CategoriesHolder dependency.
  shared_load_info.cpp
  shared_load_info.hpp
  string_set.hpp
  string_slice.cpp
  string_slice.hpp
  succinct_trie_builder.hpp
  succinct_trie_reader.hpp
  tree_structure.hpp
  trie.hpp
  trie_builder.hpp
  trie_reader.hpp
  types_mapping.cpp
  types_mapping.hpp
  unique_index.hpp
  utils.cpp
  utils.hpp
)

set(
  OTHER_FILES
  drules_struct.proto
)

file(COPY ${OTHER_FILES} DESTINATION ${CMAKE_BINARY_DIR})

#because of protobuf
add_clang_compile_options("-Wno-shorten-64-to-32" "-Wno-unused-parameter")


geocore_add_library(${PROJECT_NAME} ${SRC})
geocore_link_libraries(${PROJECT_NAME}
  base
  coding
  geometry
  platform
  succinct
  protobuf
)
geocore_add_test_subdirectory(indexer_tests)
