#pragma once

#include "base/file_name_utils.hpp"
#include "base/logging.hpp"

#include "defines.hpp"

#include <memory>
#include <string>
#include <vector>

namespace feature
{
struct GenerateInfo
{
  enum class NodeStorageType
  {
    Memory,
    Index,
    File
  };

  enum class OsmSourceType
  {
    XML,
    O5M
  };

  // Directory for .mwm.tmp files.
  std::string m_tmpDir;

  // Directory for result .mwm files.
  std::string m_targetDir;

  // Directory for all intermediate files.
  std::string m_intermediateDir;

  // Current generated file name if --output option is defined.
  std::string m_fileName;

  NodeStorageType m_nodeStorageType;
  OsmSourceType m_osmFileType;
  std::string m_osmFileName;

  std::string m_bookingDataFilename;
  std::string m_opentableDataFilename;
  std::string m_promoCatalogCitiesFilename;

  std::string m_brandsFilename;
  std::string m_brandsTranslationsFilename;

  std::string m_popularPlacesFilename;

  std::string m_idToWikidataFilename;

  std::string m_citiesBoundariesFilename;

  uint32_t m_versionDate = 0;

  std::vector<std::string> m_bucketNames;

  bool m_createWorld = false;
  bool m_haveBordersForWholeWorld = false;
  bool m_makeCoasts = false;
  bool m_emitCoasts = false;
  bool m_genAddresses = false;
  bool m_failOnCoasts = false;
  bool m_preloadCache = false;
  bool m_verbose = false;

  GenerateInfo() = default;

  void SetOsmFileType(std::string const & type)
  {
    if (type == "xml")
      m_osmFileType = OsmSourceType::XML;
    else if (type == "o5m")
      m_osmFileType = OsmSourceType::O5M;
    else
      LOG(LCRITICAL, ("Unknown source type:", type));
  }

  void SetNodeStorageType(std::string const & type)
  {
    if (type == "raw")
      m_nodeStorageType = NodeStorageType::File;
    else if (type == "map")
      m_nodeStorageType = NodeStorageType::Index;
    else if (type == "mem")
      m_nodeStorageType = NodeStorageType::Memory;
    else
      LOG(LCRITICAL, ("Incorrect node_storage type:", type));
  }

  std::string GetTmpFileName(std::string const & fileName,
                             std::string const & ext = DATA_FILE_EXTENSION_TMP) const
  {
    return base::JoinPath(m_tmpDir, fileName + ext);
  }

  std::string GetTargetFileName(std::string const & fileName,
                                std::string const & ext = DATA_FILE_EXTENSION) const
  {
    return base::JoinPath(m_targetDir, fileName + ext);
  }

  std::string GetIntermediateFileName(std::string const & fileName,
                                      std::string const & ext = "") const
  {
    return base::JoinPath(m_intermediateDir, fileName + ext);
  }

  std::string GetAddressesFileName() const
  {
    return m_genAddresses && !m_fileName.empty() ?
          GetTargetFileName(m_fileName, ADDR_FILE_EXTENSION) : "";
  }
};
}  // namespace feature
