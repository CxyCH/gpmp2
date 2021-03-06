// gpmp2 matlab wrapper declarations

// gtsam deceleration
class gtsam::Vector6;
class gtsam::Vector3;
class gtsam::Point3;
class gtsam::Pose3;
class gtsam::Point2;
class gtsam::Pose2;

class gtsam::GaussianFactorGraph;
class gtsam::Values;
virtual class gtsam::noiseModel::Base;
virtual class gtsam::NonlinearFactor;
virtual class gtsam::NonlinearFactorGraph;
virtual class gtsam::NoiseModelFactor;

namespace gpmp2 {

////////////////////////////////////////////////////////////////////////////////
// gp
////////////////////////////////////////////////////////////////////////////////

// prior factor
#include <gpmp2/gp/GaussianProcessPriorLinear.h>

// template DOF list
virtual class GaussianProcessPriorLinear : gtsam::NoiseModelFactor {
  GaussianProcessPriorLinear(size_t key1, size_t key2, size_t key3, size_t key4,
      double delta, const gtsam::noiseModel::Base* Qc_model);
};


// util class for all interpolated measurements
#include <gpmp2/gp/GaussianProcessInterpolatorLinear.h>

// template DOF list
class GaussianProcessInterpolatorLinear {
  GaussianProcessInterpolatorLinear(const gtsam::noiseModel::Base* Qc_model,
      double delta_t, double tau);
  Vector interpolatePose(Vector pose1, Vector vel1, Vector pose2, Vector vel2) const;
  Vector interpolateVelocity(Vector pose1, Vector vel1, Vector pose2, Vector vel2) const;
};



////////////////////////////////////////////////////////////////////////////////
// kinematics
////////////////////////////////////////////////////////////////////////////////


// abstract arm class use DH params
#include <gpmp2/kinematics/Arm.h>

class Arm {
  Arm(size_t dof, Vector a, Vector alpha, Vector d);
  Arm(size_t dof, Vector a, Vector alpha, Vector d, const gtsam::Pose3& base_pose);
  Arm(size_t dof, Vector a, Vector alpha, Vector d, const gtsam::Pose3& base_pose, Vector theta_bias);
  // full forward kinematics
  Matrix forwardKinematicsPose(Vector jp) const;
  Matrix forwardKinematicsPosition(Vector jp) const;
  Matrix forwardKinematicsVel(Vector jp, Vector jv) const;
  // accesses
  size_t dof() const;
  Vector a() const;
  Vector d() const;
  Vector alpha() const;
  gtsam::Pose3 base_pose() const;
};

// Abstract Point Robot class
#include <gpmp2/kinematics/PointRobot.h>

class PointRobot {
  PointRobot(size_t dof, size_t nr_links);
  // full forward kinematics
  Matrix forwardKinematicsPose(Vector jp) const;
  Matrix forwardKinematicsPosition(Vector jp) const;
  Matrix forwardKinematicsVel(Vector jp, Vector jv) const;
  // accesses
  size_t dof() const;
  size_t nr_links() const;
};


// BodySphere class
#include <gpmp2/kinematics/RobotModel.h>

class BodySphere {
  BodySphere(size_t id, double r, const gtsam::Point3& c);
};

class BodySphereVector {
  BodySphereVector();
  void push_back(const gpmp2::BodySphere& sphere);
};


// Physical ArmModel class
#include <gpmp2/kinematics/ArmModel.h>

class ArmModel {
  ArmModel(const gpmp2::Arm& arm, const gpmp2::BodySphereVector& spheres);
  // solve sphere center position in world frame
  Matrix sphereCentersMat(Vector conf) const ;
  // accesses
  size_t dof() const;
  gpmp2::Arm fk_model() const;
  size_t nr_body_spheres() const;
  double sphere_radius(size_t i) const;
};

// Point Robot Model class
#include <gpmp2/kinematics/PointRobotModel.h>

class PointRobotModel {
  PointRobotModel(const gpmp2::PointRobot& pR, const gpmp2::BodySphereVector& spheres);
  // solve sphere center position in world frame
  Matrix sphereCentersMat(Vector conf) const;
  // accesses
  size_t dof() const;
  gpmp2::PointRobot fk_model() const;
  size_t nr_body_spheres() const;
  double sphere_radius(size_t i) const;
};


// goal destination factor
#include <gpmp2/kinematics/GoalFactorArm.h>

virtual class GoalFactorArm : gtsam::NoiseModelFactor {
  GoalFactorArm(
      size_t poseKey, const gtsam::noiseModel::Base* cost_model,
      const gpmp2::Arm& arm, const gtsam::Point3& dest_point);
  //Vector evaluateError(Vector conf) const;      // for debug: plot error
};


////////////////////////////////////////////////////////////////////////////////
// obstacle
////////////////////////////////////////////////////////////////////////////////

// signed distance field class
#include <gpmp2/obstacle/SignedDistanceField.h>
class SignedDistanceField {
  SignedDistanceField(const gtsam::Point3& origin, double cell_size, size_t field_rows,
      size_t field_cols, size_t field_z);
  // insert field data
  void initFieldData(size_t z_idx, const Matrix& field_layer);
  // access
  double getSignedDistance(const gtsam::Point3& point) const;
  void print(string s) const;
};


// planar signed distance field class
#include <gpmp2/obstacle/PlanarSDF.h>
class PlanarSDF {
  PlanarSDF(const gtsam::Point2& origin, double cell_size, const Matrix& data);
  // access
  double getSignedDistance(const gtsam::Point2& point) const;
  void print(string s) const;
};


// obstacle avoid factor
#include <gpmp2/obstacle/ObstacleSDFFactorArm.h>
virtual class ObstacleSDFFactorArm : gtsam::NoiseModelFactor {
  ObstacleSDFFactorArm(
      size_t poseKey, const gpmp2::ArmModel& arm,
      const gpmp2::SignedDistanceField& sdf, double cost_sigma, double epsilon);
  Vector evaluateError(Vector pose) const;
};


// obstacle avoid factor with GP interpolation
#include <gpmp2/obstacle/ObstacleSDFFactorGPArm.h>
virtual class ObstacleSDFFactorGPArm : gtsam::NoiseModelFactor {
  ObstacleSDFFactorGPArm(
      size_t pose1Key, size_t vel1Key, size_t pose2Key, size_t vel2Key,
      const gpmp2::ArmModel& arm, const gpmp2::SignedDistanceField& sdf,
      double cost_sigma, double epsilon, const gtsam::noiseModel::Base* Qc_model,
      double delta_t, double tau);
};


// Arm obstacle avoid factor (just planar arm with 2D signed distance field)
#include <gpmp2/obstacle/ObstaclePlanarSDFFactorArm.h>

virtual class ObstaclePlanarSDFFactorArm : gtsam::NoiseModelFactor {
  ObstaclePlanarSDFFactorArm(
      size_t posekey, const gpmp2::ArmModel& arm,
      const gpmp2::PlanarSDF& sdf, double cost_sigma, double epsilon);
  Vector evaluateError(Vector pose) const;
};


// Arm obstacle avoid factor with GP  (just planar arm with 2D signed distance field)
#include <gpmp2/obstacle/ObstaclePlanarSDFFactorGPArm.h>

virtual class ObstaclePlanarSDFFactorGPArm : gtsam::NoiseModelFactor {
  ObstaclePlanarSDFFactorGPArm(
      size_t pose1key, size_t vel1key, size_t pose2key, size_t vel2key,
      const gpmp2::ArmModel& arm, const gpmp2::PlanarSDF& sdf,
      double cost_sigma, double epsilon, const gtsam::noiseModel::Base* Qc_model,
      double delta_t, double tau);
};


// planar obstacle avoid factor for Point Robot
#include <gpmp2/obstacle/ObstaclePlanarSDFFactorPointRobot.h>

virtual class ObstaclePlanarSDFFactorPointRobot : gtsam::NoiseModelFactor {
  ObstaclePlanarSDFFactorPointRobot(
      size_t posekey, const gpmp2::PointRobotModel& pR,
      const gpmp2::PlanarSDF& sdf, double cost_sigma, double epsilon);
  Vector evaluateError(Vector pose) const;
};


// planar obstacle avoid factor with GP for Point Robot 
#include <gpmp2/obstacle/ObstaclePlanarSDFFactorGPPointRobot.h>

virtual class ObstaclePlanarSDFFactorGPPointRobot : gtsam::NoiseModelFactor {
  ObstaclePlanarSDFFactorGPPointRobot(
      size_t pose1key, size_t vel1key, size_t pose2key, size_t vel2key,
      const gpmp2::PointRobotModel& pR, const gpmp2::PlanarSDF& sdf,
      double cost_sigma, double epsilon, const gtsam::noiseModel::Base* Qc_model,
      double delta_t, double tau);
};


////////////////////////////////////////////////////////////////////////////////
// planner
////////////////////////////////////////////////////////////////////////////////


/// trajectory optimizer settings
#include <gpmp2/planner/TrajOptimizerSetting.h>

class TrajOptimizerSetting {

  TrajOptimizerSetting(size_t dof);

  void set_total_step(size_t step);
  void set_total_time(double time);
  void set_epsilon(double eps);
  void set_cost_sigma(double sigma);
  void set_obs_check_inter(size_t inter);
  void set_rel_thresh(double thresh);
  void set_max_iter(size_t iter);
  void set_conf_prior_model(double sigma);
  void set_vel_prior_model(double sigma);
  void set_Qc_model(Matrix Qc);
  /// set optimization type
  void setGaussNewton();
  void setLM();
  void setDogleg();
};


/// batch trajectory optimizers
#include <gpmp2/planner/BatchTrajOptimizer.h>

/// 2D arm version optimizer
gtsam::Values BatchTrajOptimize2DArm(
    const gpmp2::ArmModel& arm, const gpmp2::PlanarSDF& sdf,
    Vector start_conf, Vector start_vel, Vector end_conf, Vector end_vel,
    const gtsam::Values& init_values, const gpmp2::TrajOptimizerSetting& setting);

/// 3D arm version optimizer
gtsam::Values BatchTrajOptimize3DArm(
    const gpmp2::ArmModel& arm, const gpmp2::SignedDistanceField& sdf,
    Vector start_conf, Vector start_vel, Vector end_conf, Vector end_vel,
    const gtsam::Values& init_values, const gpmp2::TrajOptimizerSetting& setting);



/// iSAM2 incremental trajectory optimizers
#include <gpmp2/planner/ISAM2TrajOptimizer.h>

/// 2D replanner
class ISAM2TrajOptimizer2DArm {
  ISAM2TrajOptimizer2DArm(const gpmp2::ArmModel& arm, const gpmp2::PlanarSDF& sdf,
      const gpmp2::TrajOptimizerSetting& setting);

  void initFactorGraph(Vector start_conf, Vector start_vel,
      Vector goal_conf, Vector goal_vel);
  void initValues(const gtsam::Values& init_values);
  void update();

  /// Replanning interfaces
  void changeGoalConfigAndVel(Vector goal_conf, Vector goal_vel);
  void fixConfigAndVel(size_t state_idx, Vector conf_fix, Vector vel_fix);

  /// accesses
  gtsam::Values values() const;
};

/// 3D replanner
class ISAM2TrajOptimizer3DArm {
  ISAM2TrajOptimizer3DArm(const gpmp2::ArmModel& arm, const gpmp2::SignedDistanceField& sdf,
      const gpmp2::TrajOptimizerSetting& setting);

  void initFactorGraph(Vector start_conf, Vector start_vel,
      Vector goal_conf, Vector goal_vel);
  void initValues(const gtsam::Values& init_values);
  void update();

  /// Replanning interfaces
  void changeGoalConfigAndVel(Vector goal_conf, Vector goal_vel);
  void fixConfigAndVel(size_t state_idx, Vector conf_fix, Vector vel_fix);

  /// accesses
  gtsam::Values values() const;
};


// utils for traj init and interpolation
#include <gpmp2/planner/TrajUtils.h>

/// initialization
gtsam::Values initArmTrajStraightLine(Vector init_conf, Vector end_conf, size_t total_step);

/// robot arm trajectory interpolater
gtsam::Values interpolateArmTraj(const gtsam::Values& opt_values,
    const gtsam::noiseModel::Base* Qc_model, double delta_t, size_t inter_step);


}
