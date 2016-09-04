#ifndef RWENGINE_MODELDATA_HPP
#define RWENGINE_MODELDATA_HPP
#include <stdint.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include <data/PathData.hpp>
#include <data/Model.hpp>
#ifdef RW_WINDOWS
#include <rw_mingw.hpp>
#endif

using ObjectID = uint16_t;

enum class ModelDataType
{
  SimpleInfo = 1,
  ClumpInfo = 4,
  VehicleInfo = 5,
  PedInfo = 6
};

/**
 * @brief The BaseModelData struct stores data common to all types
 */
class BaseModelData
{
public:
  ObjectID id;
  const ModelDataType type;
  std::string modelName;
  std::string textureName;

  /// @todo replace with correct data
  Model* model = nullptr;
  ModelFrame* frames[3] = {};

  BaseModelData(const ModelDataType type) : type(type)
  {
  }

  virtual ~BaseModelData()
  {
  }
};

typedef std::shared_ptr<BaseModelData> ObjectInformationPtr;

class SimpleModelData : public BaseModelData {
public:
  static constexpr ModelDataType kType = ModelDataType::SimpleInfo;

  SimpleModelData() : BaseModelData(kType)
  {
  }

  uint8_t numClumps;
  float drawDistance[3];
  int32_t flags;
  bool LOD;

  short timeOn;
  short timeOff;

  enum {
    NORMAL_CULL = 1,  /// Cull model if player doesn't look at it. Ignored in GTA 3.
    DO_NOT_FADE =
        1 << 1,  /// Do not fade the object when it is being loaded into or out of view.
    DRAW_LAST = 1 << 2,  /// Model is transparent. Render this object after all opaque
                         /// objects, allowing transparencies of other objects to be
                         /// visible through this object.
    ADDITIVE =
        1 << 3,  /// Render with additive blending. Previous flag must be enabled too.
    IS_SUBWAY = 1 << 4,  /// Model is a tunnel, i.e. set the object as invisible unless
                         /// the player enters cull zone flag 128. This flag works only
                         /// with static models.
    IGNORE_LIGHTING =
        1 << 5,  /// Don't use static lighting, we want dynamic if it's possible.
    NO_ZBUFFER_WRITE = 1 << 6,  /// Model is a shadow. Disable writing to z-buffer when
                                /// rendering it, allowing transparencies of other
                                /// objects, shadows, and lights to be visible through
                                /// this object. (Not implemented in the PS2 version)
  };

  // Information loaded from PATH sections
  std::vector<PathData> paths;
};

typedef std::shared_ptr<SimpleModelData> ObjectDataPtr;

/**
 * @todo this should be something like ClumpModelData
 */
class CutsceneModelData : public BaseModelData {
public:
  static constexpr ModelDataType kType = ModelDataType::ClumpInfo;

  CutsceneModelData() : BaseModelData(kType)
  {
  }

};

struct CharacterModelData : public BaseModelData {
public:
  static constexpr ModelDataType kType = ModelDataType::PedInfo;

  CharacterModelData() : BaseModelData(kType)
  {
  }

  std::string type;
  std::string behaviour;
  std::string animGroup;
  uint8_t driveMask;
};

class VehicleModelData : public BaseModelData {
public:
  static constexpr ModelDataType kType = ModelDataType::VehicleInfo;

  VehicleModelData() : BaseModelData(kType)
  {
  }

  enum VehicleClass {
    IGNORE = 0,
    NORMAL = 1,
    POORFAMILY = 1 << 1,
    RICHFAMILY = 1 << 2,
    EXECUTIVE = 1 << 3,
    WORKER = 1 << 4,
    BIG = 1 << 5,
    TAXI = 1 << 6,
    MOPED = 1 << 7,
    MOTORBIKE = 1 << 8,
    LEISUREBOAT = 1 << 9,
    WORKERBOAT = 1 << 10,
    BICYCLE = 1 << 11,
    ONFOOT = 1 << 12,
  };

  enum VehicleType {
    CAR,
    BOAT,
    TRAIN,
    PLANE,
    HELI,
  };

  VehicleType vehicletype;
  std::string handlingID;
  std::string gameName;
  VehicleClass classType;
  uint8_t frequency;  // big enough int type?
  uint8_t lvl;        // big enough int type?
  uint16_t comprules;
  union {                   // big enough int types?
    uint16_t wheelModelID;  // used only when type == CAR
    int16_t modelLOD;       // used only when type == PLANE
  };
  float wheelScale;  // used only when type == CAR
};

typedef std::shared_ptr<VehicleModelData> VehicleDataHandle;

/**
 * This is orthogonal to object class, it gives
 * Instances different physical properties.
 */
struct DynamicObjectData {
  std::string modelName;
  float mass;        // Kg
  float turnMass;    // Kg m^3
  float airRes;      // fraction
  float elacticity;  // "
  float bouancy;
  float uprootForce;  // Force
  float collDamageMulti;
  /*
   * 1: change model
   * 2: split model
   * 3: smash
   * 4: change and smash
   */
  uint8_t collDamageFlags;
  /*
   * 1: lampost
   * 2: smallbox
   * 3: bigbox
   * 4: fencepart
   */
  uint8_t collResponseFlags;
  bool cameraAvoid;
};

#endif
