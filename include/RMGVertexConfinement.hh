// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef _RMG_VERTEX_CONFINEMENT_HH_
#define _RMG_VERTEX_CONFINEMENT_HH_

#include <chrono>
#include <optional>
#include <queue>
#include <regex>
#include <vector>

#include "G4AutoLock.hh"
#include "G4GenericMessenger.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4UnitsTable.hh"

#include "RMGVVertexGenerator.hh"

class G4VPhysicalVolume;
class G4VSolid;

/** @brief Class for generating vertices in physical or geometrical volumes.
 */
class RMGVertexConfinement : public RMGVVertexGenerator {

  public:

    /** @brief Different types of geometrical (user) defined solids. */
    enum class GeometricalSolidType {
      kSphere,
      kCylinder,
      kBox,
    };

    /** @brief Information about the geometrical (user) defined solids. */
    struct GenericGeometricalSolidData {
        GeometricalSolidType solid_type = GeometricalSolidType::kBox;
        G4ThreeVector volume_center = G4ThreeVector(0, 0, 0);
        double sphere_inner_radius = 0;
        double sphere_outer_radius = -1;
        double cylinder_inner_radius = 0;
        double cylinder_outer_radius = -1;
        double cylinder_height = -1;
        double cylinder_starting_angle = 0;
        double cylinder_spanning_angle = CLHEP::twopi;
        double box_x_length = -1;
        double box_y_length = -1;
        double box_z_length = -1;
    };

    /**
     * @brief Strategy for sampling physical and geometrical volumes.
     *
     * @details Can be either:
     * - @c kIntersectPhysicalWithGeometrical : In which case vertices are generated in the
     * intersection of the set of physical and geometrical volumes.
     * - @c kUnionAll Generate in the union of all volumes, weighted by surface area / volume.
     * - @c kSubtractGeometrical : Similar to @c kIntersectPhysicalWithGeometrical but specified
     * regions can also be excluded.
     */
    enum class SamplingMode {
      kIntersectPhysicalWithGeometrical,
      kUnionAll,
      kSubtractGeometrical,
    };

    /** @brief Types of volume to sample, either physical (a volume in the geometry), geometrical
     * (defined by the user) or unset. */
    enum class VolumeType {
      kPhysical,
      kGeometrical,
      kUnset,
    };

    RMGVertexConfinement();

    void BeginOfRunAction(const G4Run* run) override;
    void EndOfRunAction(const G4Run* run) override;

    /** @brief Generate the actual vertex, according to the sampling mode (see \ref
     * RMGVertexConfinement::SamplingMode). */
    bool GenerateVertex(G4ThreeVector& v) override;

    /**
     * This function is used by the messenger command to add a physical
     * volume(s) to the list of volumes to consider for sampling.
     *
     * @param name The name of the physical volume or a regular expression
     * supported by \ref std::regex
     * @param copy_nr The copy number of the physical volume or a regular
     * expression supported by \ref std::regex
     */
    void AddPhysicalVolumeNameRegex(std::string name, std::string copy_nr = ".*");

    inline void AddGeometricalVolume(GenericGeometricalSolidData& data) {
      fGeomVolumeData.emplace_back(data);
    }
    void Reset();

    inline void SetSamplingMode(SamplingMode mode) { fSamplingMode = mode; }
    inline void SetFirstSamplingVolumeType(VolumeType type) { fFirstSamplingVolumeType = type; }

    inline std::vector<GenericGeometricalSolidData>& GetGeometricalSolidDataList() {
      return fGeomVolumeData;
    }

    /**
     * An object which we can generate position samples in. Based on either a
     * @c G4VPhysicalVolume or geometrical volume defined by a @c G4VSolid . The
     * sampling can be performed either on the surface or in the volume of the solid.
     *
     * This structure must contain at least a non-null pointer, between the @c physical_volume and
     * @c sampling_solid arguments. The idea is that:
     *  - physical volumes get always a bounding box assigned, but at later time
     *  - purely geometrical volumes only have the sampling_solid member defined
     */
    struct SampleableObject {

        SampleableObject() = default;
        SampleableObject(const SampleableObject&) = default;

        /**
         * @brief SampleableObject constructor.
         *
         * @param physvol The physical volume.
         * @param rot A rotation matrix for the sampling solid.
         * @param trans A translation vector for the sampling solid.
         * @param solid A solid for geometrical volume sampling or for generating candidate points
         * or rejection sampling.
         * @param is_native_sampleable A flag of whether the solid is natively sampeable.
         * @param on_surface A flag of whether the solid should be sampled on the surface.
         */
        SampleableObject(
            G4VPhysicalVolume* physvol,
            G4RotationMatrix rot,
            G4ThreeVector trans,
            G4VSolid* solid,
            bool is_native_sampleable = false,
            bool on_surface = false
        );

        // NOTE: G4 volume/solid pointers should be fully owned by G4, avoid trying to delete them
        ~SampleableObject() = default;

        /**
         * @brief Check if the vertex is inside the solid.
         * @param vertex The sampled vertex.
         * @returns Boolean flag of whether the vertexx is inside the solid.
         */
        [[nodiscard]] bool IsInside(const G4ThreeVector& vertex) const;

        /**
         * @brief Generate a sample from the solid.
         *
         * @details Depending on if the solid is a basic one either sample natively,
         * or using rejection sampling. Either samples the volume or the surface depending
         * on the @c surface_sample member.
         * - For surface sampling mode the solid is either natively sampled (if this is
         * implemented), or is sampled with \ref SampleableObject::GenerateSurfacePoint
         * - For volume sampling, if the solid is not natively sampleable, points are generated in a
         * bounding box and then rejection sampling is used using \ref SampleableObject::IsInside.
         *
         * @param vertex The sampled vertex.
         * @param max_attempts The maximum number of candidate vertices for rejection sampling.
         * @param force_containment_check Whether to force a check on where the point is
         * inside the solid.
         * @param n_trials The total number of trials performed.
         */
        [[nodiscard]] bool Sample(
            G4ThreeVector& vertex,
            size_t max_attempts,
            bool force_containment_check,
            size_t& n_trials
        ) const;

        /**
         * @brief Generate a point on the surface of the solid.
         *
         * @details This follows the algorithm from https://arxiv.org/abs/0802.2960.
         * - Produce a direction vector corresponding to a uniform flux in a bounding sphere.
         * - Find the intersections of this line with the solid.
         * - Pick one intersection, or repeat.
         *
         * @param vertex The sampled vertex,
         * @param max_attempts The maximum number of attempts to find a valid vertex.
         * @param n_max The maximum number of intersections possible for the solid,
         * can be an overestimate.
         */
        [[nodiscard]] bool GenerateSurfacePoint(
            G4ThreeVector& vertex,
            size_t max_attempts,
            size_t n_max
        ) const;

        // methods for the generic surface sampling
        /**
         * @brief Get the number of intersections between the solid and the line starting at @c
         * start with direction @c dir.
         *
         * @details This is used in the generic surface sampling algorithm. This function makes use
         * of the methods @c GetDistanceToIn(p,v) and @c GetDistanceToOut(p,v) of @c G4VSolid .
         * It continually looks for the distance to the next boundary (along the line)
         * until this becomes zero indicating there are no more intersections.
         *
         * @param start The starting vector of the line, note this should be outside the solid.
         * @param dir   The direction vector.
         *
         * @returns A vector of the points of intersection.
         */
        [[nodiscard]] std::vector<G4ThreeVector> GetIntersections(
            G4ThreeVector start,
            G4ThreeVector dir
        ) const;

        /**
         * @brief Get a position and direction for the generic surface sampling algorithm.
         *
         * @details This generates a point on a bounding sphere, then shifts by some impact
         * parameter, following the algorithm from https://arxiv.org/abs/0802.2960. This produces a
         * uniform and isotropic flux inside the bounding sphere.
         *
         * @param dir The direction vector for the point.
         * @param pos The initial position for the point.
         */
        void GetDirection(G4ThreeVector& dir, G4ThreeVector& pos) const;

        G4VPhysicalVolume* physical_volume = nullptr;
        G4VSolid* sampling_solid = nullptr;
        G4RotationMatrix rotation;
        G4ThreeVector translation;

        double volume = -1;
        double surface = -1;

        bool surface_sample = false;
        bool native_sample = false;
        int max_num_intersections = 10;
    };

    /** A collection of @c SampleableObjects . Can be used
     * to sample from by selecting a volume weighted by surface area
     * or volume.
     */
    struct SampleableObjectCollection {

        SampleableObjectCollection() = default;
        inline ~SampleableObjectCollection() { data.clear(); }

        /** @brief Select a @c SampleableObject from the collection, weighted by surface area.
         *  @returns a reference to the chosen @c SampleableObject .
         */
        [[nodiscard]] const SampleableObject& SurfaceWeightedRand() const;

        /** @brief Select a @c SampleableObject from the collection, weighted by volume.
         *  @returns a reference to the chosen @c SampleableObject .
         */
        [[nodiscard]] const SampleableObject& VolumeWeightedRand() const;
        [[nodiscard]] bool IsInside(const G4ThreeVector& vertex) const;

        // emulate std::vector
        [[nodiscard]] size_t size() const { return data.size(); }
        SampleableObject& at(size_t i) { return data.at(i); }
        template<typename... Args> void emplace_back(Args&&... args);
        [[nodiscard]] inline bool empty() const { return data.empty(); }
        inline SampleableObject& back() { return data.back(); }
        inline void clear() { data.clear(); }
        inline void insert(SampleableObjectCollection& other) {
          for (size_t i = 0; i < other.size(); ++i) this->emplace_back(other.at(i));
        }

        std::vector<SampleableObject> data = {};
        double total_volume = 0;
        double total_surface = 0;
    };

  private:

    struct VolumeTreeEntry {
        VolumeTreeEntry() = delete;
        VolumeTreeEntry(const VolumeTreeEntry&) = default;
        inline VolumeTreeEntry(G4VPhysicalVolume* pv) { physvol = pv; }

        G4VPhysicalVolume* physvol;

        G4ThreeVector vol_global_translation; // origin
        G4RotationMatrix vol_global_rotation; // identity
        std::vector<G4RotationMatrix> partial_rotations;
        std::vector<G4ThreeVector> partial_translations;
    };

    void InitializePhysicalVolumes();
    void InitializeGeometricalVolumes(bool use_excluded_volumes);
    bool ActualGenerateVertex(G4ThreeVector& v);

    std::vector<std::string> fPhysicalVolumeNameRegexes;
    std::vector<std::string> fPhysicalVolumeCopyNrRegexes;

    std::vector<GenericGeometricalSolidData> fGeomVolumeData;
    std::vector<GenericGeometricalSolidData> fExcludedGeomVolumeData;

    static G4Mutex fGeometryMutex;
    // the final geometry data is shared between all threads and protected by fGeometryMutex.
    // this is to prevent problems in G4SolidStore, which is apparently not safe to mutate from
    // multiple threads. note that some operations that just appear to read static data might also
    // mutate the G4SolidStore temporarily, e.g. G4SubstractionSolid::GetCubicVolume()
    static SampleableObjectCollection fPhysicalVolumes;
    static SampleableObjectCollection fGeomVolumeSolids;
    static SampleableObjectCollection fExcludedGeomVolumeSolids;

    static bool fVolumesInitialized;

    SamplingMode fSamplingMode = SamplingMode::kUnionAll;
    VolumeType fFirstSamplingVolumeType = VolumeType::kUnset;

    bool fOnSurface = false;
    bool fForceContainmentCheck = false;
    bool fLastSolidExcluded = false;
    int fSurfaceSampleMaxIntersections = -1;

    // counters used for the current run.
    size_t fTrials = 0;
    std::chrono::nanoseconds fVertexGenerationTime{};

    std::vector<std::unique_ptr<G4GenericMessenger>> fMessengers;
    void SetSamplingModeString(std::string mode);
    void SetFirstSamplingVolumeTypeString(std::string type);

    void AddGeometricalVolumeString(std::string solid);
    void AddExcludedGeometricalVolumeString(std::string solid);

    GenericGeometricalSolidData& SafeBack(
        std::optional<GeometricalSolidType> solid_type = std::nullopt
    );

    // FIXME: there is no easy way to set the position vector all at once with
    // G4GenericMessenger. Only ::DeclarePropertyWithUnit() accepts vectors and
    // one cannot call functions with more than 2 arguments (3 coordinated + 1
    // units needed). Ugly!
    inline void SetGeomVolumeCenter(const G4ThreeVector& v) { this->SafeBack().volume_center = v; }
    inline void SetGeomVolumeCenterX(double x) { this->SafeBack().volume_center.setX(x); }
    inline void SetGeomVolumeCenterY(double y) { this->SafeBack().volume_center.setY(y); }
    inline void SetGeomVolumeCenterZ(double z) { this->SafeBack().volume_center.setZ(z); }

    inline void SetGeomSphereInnerRadius(double r) {
      this->SafeBack(GeometricalSolidType::kSphere).sphere_inner_radius = r;
    }
    inline void SetGeomSphereOuterRadius(double r) {
      this->SafeBack(GeometricalSolidType::kSphere).sphere_outer_radius = r;
    }

    inline void SetGeomCylinderInnerRadius(double r) {
      this->SafeBack(GeometricalSolidType::kCylinder).cylinder_inner_radius = r;
    }
    inline void SetGeomCylinderOuterRadius(double r) {
      this->SafeBack(GeometricalSolidType::kCylinder).cylinder_outer_radius = r;
    }
    inline void SetGeomCylinderHeight(double h) {
      this->SafeBack(GeometricalSolidType::kCylinder).cylinder_height = h;
    }
    inline void SetGeomCylinderStartingAngle(double a) {
      this->SafeBack(GeometricalSolidType::kCylinder).cylinder_starting_angle = a;
    }
    inline void SetGeomCylinderSpanningAngle(double a) {
      this->SafeBack(GeometricalSolidType::kCylinder).cylinder_spanning_angle = a;
    }

    inline void SetGeomBoxXLength(double x) {
      this->SafeBack(GeometricalSolidType::kBox).box_x_length = x;
    }
    inline void SetGeomBoxYLength(double y) {
      this->SafeBack(GeometricalSolidType::kBox).box_y_length = y;
    }
    inline void SetGeomBoxZLength(double z) {
      this->SafeBack(GeometricalSolidType::kBox).box_z_length = z;
    }

    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
