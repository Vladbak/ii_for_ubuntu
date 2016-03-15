//
//  Material.h
//  libraries/model/src/model
//
//  Created by Sam Gateau on 12/10/2014.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#ifndef hifi_model_Material_h
#define hifi_model_Material_h

#include <bitset>
#include <map>

#include <ColorUtils.h>

#include <gpu/Resource.h>

namespace model {

class TextureMap;
typedef std::shared_ptr< TextureMap > TextureMapPointer;

// Material Key is a coarse trait description of a material used to classify the materials
class MaterialKey {
public:
   enum FlagBit {
        EMISSIVE_VAL_BIT = 0,
        ALBEDO_VAL_BIT,
        METALLIC_VAL_BIT,
        GLOSSY_VAL_BIT,
        TRANSPARENT_VAL_BIT,

        EMISSIVE_MAP_BIT,
        ALBEDO_MAP_BIT,
        METALLIC_MAP_BIT,
        ROUGHNESS_MAP_BIT,
        TRANSPARENT_MAP_BIT,
        NORMAL_MAP_BIT,
        OCCLUSION_MAP_BIT,
        LIGHTMAP_MAP_BIT,

        NUM_FLAGS,
    };
    typedef std::bitset<NUM_FLAGS> Flags;

    enum MapChannel {
        EMISSIVE_MAP = 0,
        ALBEDO_MAP,
        METALLIC_MAP,
        ROUGHNESS_MAP,
        TRANSPARENT_MAP,
        NORMAL_MAP,
        OCCLUSION_MAP,
        LIGHTMAP_MAP,

        NUM_MAP_CHANNELS,
    };

    // The signature is the Flags
    Flags _flags;

    MaterialKey() : _flags(0) {}
    MaterialKey(const Flags& flags) : _flags(flags) {}

    class Builder {
        Flags _flags{ 0 };
    public:
        Builder() {}

        MaterialKey build() const { return MaterialKey(_flags); }

        Builder& withEmissive() { _flags.set(EMISSIVE_VAL_BIT); return (*this); }
        Builder& withAlbedo() { _flags.set(ALBEDO_VAL_BIT); return (*this); }
        Builder& withMetallic() { _flags.set(METALLIC_VAL_BIT); return (*this); }
        Builder& withGlossy() { _flags.set(GLOSSY_VAL_BIT); return (*this); }
        Builder& withTransparent() { _flags.set(TRANSPARENT_VAL_BIT); return (*this); }

        Builder& withEmissiveMap() { _flags.set(EMISSIVE_MAP_BIT); return (*this); }
        Builder& withAlbedoMap() { _flags.set(ALBEDO_MAP_BIT); return (*this); }
        Builder& withMetallicMap() { _flags.set(METALLIC_MAP_BIT); return (*this); }
        Builder& withRoughnessMap() { _flags.set(ROUGHNESS_MAP_BIT); return (*this); }
        Builder& withTransparentMap() { _flags.set(TRANSPARENT_MAP_BIT); return (*this); }

        Builder& withNormalMap() { _flags.set(NORMAL_MAP_BIT); return (*this); }
        Builder& withOcclusionMap() { _flags.set(OCCLUSION_MAP_BIT); return (*this); }
        Builder& withLightmapMap() { _flags.set(LIGHTMAP_MAP_BIT); return (*this); }

        // Convenient standard keys that we will keep on using all over the place
        static MaterialKey opaqueAlbedo() { return Builder().withAlbedo().build(); }
    };

    void setEmissive(bool value) { _flags.set(EMISSIVE_VAL_BIT, value); }
    bool isEmissive() const { return _flags[EMISSIVE_VAL_BIT]; }

    void setEmissiveMap(bool value) { _flags.set(EMISSIVE_MAP_BIT, value); }
    bool isEmissiveMap() const { return _flags[EMISSIVE_MAP_BIT]; }
 
    void setAlbedo(bool value) { _flags.set(ALBEDO_VAL_BIT, value); }
    bool isAlbedo() const { return _flags[ALBEDO_VAL_BIT]; }

    void setAlbedoMap(bool value) { _flags.set(ALBEDO_MAP_BIT, value); }
    bool isAlbedoMap() const { return _flags[ALBEDO_MAP_BIT]; }

    void setMetallic(bool value) { _flags.set(METALLIC_VAL_BIT, value); }
    bool isMetallic() const { return _flags[METALLIC_VAL_BIT]; }

    void setMetallicMap(bool value) { _flags.set(METALLIC_MAP_BIT, value); }
    bool isMetallicMap() const { return _flags[METALLIC_MAP_BIT]; }

    void setGlossy(bool value) { _flags.set(GLOSSY_VAL_BIT, value); }
    bool isGlossy() const { return _flags[GLOSSY_VAL_BIT]; }
    bool isRough() const { return !_flags[GLOSSY_VAL_BIT]; }

    void setRoughnessMap(bool value) { _flags.set(ROUGHNESS_MAP_BIT, value); }
    bool isRoughnessMap() const { return _flags[ROUGHNESS_MAP_BIT]; }

    void setTransparent(bool value) { _flags.set(TRANSPARENT_VAL_BIT, value); }
    bool isTransparent() const { return _flags[TRANSPARENT_VAL_BIT]; }
    bool isOpaque() const { return !_flags[TRANSPARENT_VAL_BIT]; }

    void setTransparentMap(bool value) { _flags.set(TRANSPARENT_MAP_BIT, value); }
    bool isTransparentMap() const { return _flags[TRANSPARENT_MAP_BIT]; }

    void setNormalMap(bool value) { _flags.set(NORMAL_MAP_BIT, value); }
    bool isNormalMap() const { return _flags[NORMAL_MAP_BIT]; }

    void setOcclusionMap(bool value) { _flags.set(OCCLUSION_MAP_BIT, value); }
    bool isOcclusionMap() const { return _flags[OCCLUSION_MAP_BIT]; }

    void setLightmapMap(bool value) { _flags.set(LIGHTMAP_MAP_BIT, value); }
    bool isLightmapMap() const { return _flags[LIGHTMAP_MAP_BIT]; }

    void setMapChannel(MapChannel channel, bool value) { _flags.set(EMISSIVE_MAP_BIT + channel, value); }
    bool isMapChannel(MapChannel channel) const { return _flags[EMISSIVE_MAP_BIT + channel]; }

};


class MaterialFilter {
public:
    MaterialKey::Flags _value{ 0 };
    MaterialKey::Flags _mask{ 0 };


    MaterialFilter(const MaterialKey::Flags& value = MaterialKey::Flags(0), const MaterialKey::Flags& mask = MaterialKey::Flags(0)) : _value(value), _mask(mask) {}

    class Builder {
        MaterialKey::Flags _value{ 0 };
        MaterialKey::Flags _mask{ 0 };
    public:
        Builder() {}

        MaterialFilter build() const { return MaterialFilter(_value, _mask); }

        Builder& withoutEmissive()       { _value.reset(MaterialKey::EMISSIVE_VAL_BIT); _mask.set(MaterialKey::EMISSIVE_VAL_BIT); return (*this); }
        Builder& withEmissive()        { _value.set(MaterialKey::EMISSIVE_VAL_BIT);  _mask.set(MaterialKey::EMISSIVE_VAL_BIT); return (*this); }

        Builder& withoutEmissiveMap()       { _value.reset(MaterialKey::EMISSIVE_MAP_BIT); _mask.set(MaterialKey::EMISSIVE_MAP_BIT); return (*this); }
        Builder& withEmissiveMap()        { _value.set(MaterialKey::EMISSIVE_MAP_BIT);  _mask.set(MaterialKey::EMISSIVE_MAP_BIT); return (*this); }

        Builder& withoutAlbedo()       { _value.reset(MaterialKey::ALBEDO_VAL_BIT); _mask.set(MaterialKey::ALBEDO_VAL_BIT); return (*this); }
        Builder& withAlbedo()        { _value.set(MaterialKey::ALBEDO_VAL_BIT);  _mask.set(MaterialKey::ALBEDO_VAL_BIT); return (*this); }

        Builder& withoutAlbedoMap()       { _value.reset(MaterialKey::ALBEDO_MAP_BIT); _mask.set(MaterialKey::ALBEDO_MAP_BIT); return (*this); }
        Builder& withAlbedoMap()        { _value.set(MaterialKey::ALBEDO_MAP_BIT);  _mask.set(MaterialKey::ALBEDO_MAP_BIT); return (*this); }

        Builder& withoutMetallic()       { _value.reset(MaterialKey::METALLIC_VAL_BIT); _mask.set(MaterialKey::METALLIC_VAL_BIT); return (*this); }
        Builder& withMetallic()        { _value.set(MaterialKey::METALLIC_VAL_BIT);  _mask.set(MaterialKey::METALLIC_VAL_BIT); return (*this); }

        Builder& withoutMetallicMap()       { _value.reset(MaterialKey::METALLIC_MAP_BIT); _mask.set(MaterialKey::METALLIC_MAP_BIT); return (*this); }
        Builder& withMetallicMap()        { _value.set(MaterialKey::METALLIC_MAP_BIT);  _mask.set(MaterialKey::METALLIC_MAP_BIT); return (*this); }

        Builder& withoutGlossy()       { _value.reset(MaterialKey::GLOSSY_VAL_BIT); _mask.set(MaterialKey::GLOSSY_VAL_BIT); return (*this); }
        Builder& withGlossy()        { _value.set(MaterialKey::GLOSSY_VAL_BIT);  _mask.set(MaterialKey::GLOSSY_VAL_BIT); return (*this); }

        Builder& withoutRoughnessMap()       { _value.reset(MaterialKey::ROUGHNESS_MAP_BIT); _mask.set(MaterialKey::ROUGHNESS_MAP_BIT); return (*this); }
        Builder& withRoughnessMap()        { _value.set(MaterialKey::ROUGHNESS_MAP_BIT);  _mask.set(MaterialKey::ROUGHNESS_MAP_BIT); return (*this); }

        Builder& withoutTransparent()       { _value.reset(MaterialKey::TRANSPARENT_VAL_BIT); _mask.set(MaterialKey::TRANSPARENT_VAL_BIT); return (*this); }
        Builder& withTransparent()        { _value.set(MaterialKey::TRANSPARENT_VAL_BIT);  _mask.set(MaterialKey::TRANSPARENT_VAL_BIT); return (*this); }

        Builder& withoutTransparentMap()       { _value.reset(MaterialKey::TRANSPARENT_MAP_BIT); _mask.set(MaterialKey::TRANSPARENT_MAP_BIT); return (*this); }
        Builder& withTransparentMap()        { _value.set(MaterialKey::TRANSPARENT_MAP_BIT);  _mask.set(MaterialKey::TRANSPARENT_MAP_BIT); return (*this); }

        Builder& withoutNormalMap()       { _value.reset(MaterialKey::NORMAL_MAP_BIT); _mask.set(MaterialKey::NORMAL_MAP_BIT); return (*this); }
        Builder& withNormalMap()        { _value.set(MaterialKey::NORMAL_MAP_BIT);  _mask.set(MaterialKey::NORMAL_MAP_BIT); return (*this); }

        Builder& withoutOcclusionMap()       { _value.reset(MaterialKey::OCCLUSION_MAP_BIT); _mask.set(MaterialKey::OCCLUSION_MAP_BIT); return (*this); }
        Builder& withOcclusionMap()        { _value.set(MaterialKey::OCCLUSION_MAP_BIT);  _mask.set(MaterialKey::OCCLUSION_MAP_BIT); return (*this); }

        Builder& withoutLightmapMap()       { _value.reset(MaterialKey::LIGHTMAP_MAP_BIT); _mask.set(MaterialKey::LIGHTMAP_MAP_BIT); return (*this); }
        Builder& withLightmapMap()        { _value.set(MaterialKey::LIGHTMAP_MAP_BIT);  _mask.set(MaterialKey::LIGHTMAP_MAP_BIT); return (*this); }

        // Convenient standard keys that we will keep on using all over the place
        static MaterialFilter opaqueAlbedo() { return Builder().withAlbedo().withoutTransparent().build(); }
    };

    // Item Filter operator testing if a key pass the filter
    bool test(const MaterialKey& key) const { return (key._flags & _mask) == (_value & _mask); }

    class Less {
    public:
        bool operator() (const MaterialFilter& left, const MaterialFilter& right) const {
            if (left._value.to_ulong() == right._value.to_ulong()) {
                return left._mask.to_ulong() < right._mask.to_ulong();
            } else {
                return left._value.to_ulong() < right._value.to_ulong();
            }
        }
    };
};

class Material {
public:
    typedef gpu::BufferView UniformBufferView;

    typedef glm::vec3 Color;

    typedef MaterialKey::MapChannel MapChannel;
    typedef std::map<MapChannel, TextureMapPointer> TextureMaps;
    typedef std::bitset<MaterialKey::NUM_MAP_CHANNELS> MapFlags;

    Material();
    Material(const Material& material);
    Material& operator= (const Material& material);
    virtual ~Material();

    const MaterialKey& getKey() const { return _key; }

    void setEmissive(const Color& emissive, bool isSRGB = true);
    Color getEmissive(bool SRGB = true) const { return (SRGB ? ColorUtils::toGamma22Vec3(_schemaBuffer.get<Schema>()._emissive) : _schemaBuffer.get<Schema>()._emissive); }

    void setOpacity(float opacity);
    float getOpacity() const { return _schemaBuffer.get<Schema>()._opacity; }

    void setAlbedo(const Color& albedo, bool isSRGB = true);
    Color getAlbedo(bool SRGB = true) const { return (SRGB ? ColorUtils::toGamma22Vec3(_schemaBuffer.get<Schema>()._albedo) : _schemaBuffer.get<Schema>()._albedo); }

    void setFresnel(const Color& fresnel, bool isSRGB = true);
    Color getFresnel(bool SRGB = true) const { return (SRGB ? ColorUtils::toGamma22Vec3(_schemaBuffer.get<Schema>()._fresnel) : _schemaBuffer.get<Schema>()._fresnel); }

    void setMetallic(float metallic);
    float getMetallic() const { return _schemaBuffer.get<Schema>()._metallic; }

    void setRoughness(float roughness);
    float getRoughness() const { return _schemaBuffer.get<Schema>()._roughness; }

    // Schema to access the attribute values of the material
    class Schema {
    public:
        glm::vec3 _emissive{ 0.0f }; // No Emissive
        float _opacity{ 1.0f }; // Opacity = 1 => Not Transparent

        glm::vec3 _albedo{ 0.5f }; // Grey albedo => isAlbedo
        float _roughness{ 1.0f }; // Roughness = 1 => Not Glossy

        glm::vec3 _fresnel{ 0.03f }; // Fresnel value for a default non metallic
        float _metallic{ 0.0f }; // Not Metallic


        glm::vec3 _spare0{ 0.0f };

        uint32_t _key{ 0 }; // a copy of the materialKey

        // for alignment beauty, Material size == Mat4x4

        Schema() {}
    };

    const UniformBufferView& getSchemaBuffer() const { return _schemaBuffer; }

    // The texture map to channel association
    void setTextureMap(MapChannel channel, const TextureMapPointer& textureMap);
    const TextureMaps& getTextureMaps() const { return _textureMaps; }

    // conversion from legacy material properties to PBR equivalent
    static float shininessToRoughness(float shininess) { return 1.0f - shininess / 100.0f; }

protected:

    MaterialKey _key;
    UniformBufferView _schemaBuffer;
    TextureMaps _textureMaps;

};
typedef std::shared_ptr< Material > MaterialPointer;

};

#endif
