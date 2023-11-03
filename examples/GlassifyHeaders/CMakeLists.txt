set(PROJECT_NAME GlassifyHeaders)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "../../include/glas_decl.h"
    "../../include/glas_dependencies.h"
    "../../include/glas_enum.h"
    "../../include/glas_impl.h"
    "../../include/glas_properties.h"
    "../../include/glassify.h"
    "../../include/serialization/glas_serialization_binary.h"
    "../../include/serialization/glas_serialization_config_binary.h"
    "../../include/serialization/glas_serialization_config_json.h"
    "../../include/serialization/glas_serialization_config_yaml.h"
    "../../include/serialization/glas_serialization_json.h"
    "../../include/serialization/glas_serialization_yaml.h"
    "../../include/storage/glas_storage.h"
    "../../include/storage/glas_storage_config.h"
)
source_group("Header Files" FILES ${Header_Files})

set(ALL_FILES
    ${Header_Files}
)

add_custom_target(${PROJECT_NAME} SOURCES ${ALL_FILES})