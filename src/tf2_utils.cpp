// Copyright (C) 2023 Thies Lennart Alff
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
// USA

#include "hippo_common/tf2_utils.hpp"

namespace hippo_common {
namespace tf2_utils {

static const Eigen::Quaterniond q_enu_ned{
    EulerToQuaternion(kPi, 0.0, kPi / 2.0)};
static const Eigen::Quaterniond q_flu_frd{EulerToQuaternion(kPi, 0.0, 0.0)};

Eigen::Quaterniond EulerToQuaternion(double _roll, double _pitch, double _yaw) {
  Eigen::Quaterniond q;
  q = Eigen::AngleAxisd(_yaw, Eigen::Vector3d::UnitZ()) *
      Eigen::AngleAxisd(_pitch, Eigen::Vector3d::UnitY()) *
      Eigen::AngleAxisd(_roll, Eigen::Vector3d::UnitX());
  return q;
}

/**
 * @brief Calculates the orientation for a given desired heading and a roll
 * angle.
 *
 * @param _heading Desired heading expressed as a unit vector.
 * @param _roll Desired roll angle in [rad].
 * @return Eigen::Quaterniond Orientation
 */
Eigen::Quaterniond QuaternionFromHeading(const Eigen::Vector3d &_heading,
                                         double _roll) {
  const Eigen::Vector3d z_axis_intermediate{Eigen::AngleAxisd{_roll, _heading} *
                                            Eigen::Vector3d::UnitZ()};
  const Eigen::Vector3d y_axis_desired =
      z_axis_intermediate.cross(_heading).normalized();
  const Eigen::Vector3d z_axis_desired =
      _heading.cross(y_axis_desired).normalized();

  Eigen::Matrix3d R;
  for (int i = 0; i < 3; ++i) {
    R(i, 0) = _heading(i);
    R(i, 1) = y_axis_desired(i);
    R(i, 2) = z_axis_desired(i);
  }
  Eigen::Quaterniond attitude{R};
  return attitude;
}

Eigen::Quaterniond RotationBetweenNormalizedVectors(
    const Eigen::Vector3d &_v1, const Eigen::Vector3d &_v2) {
  // https://stackoverflow.com/questions/1171849/finding-quaternion-representing-the-rotation-from-one-vector-to-another
  Eigen::Vector3d n = _v1.cross(_v2);
  Eigen::Quaterniond q;
  q.x() = n.x();
  q.y() = n.y();
  q.z() = n.z();
  q.w() = 1 + _v1.dot(_v2);
  q.normalize();
  return q;
}

geometry_msgs::msg::Transform ENUtoNED() {
  geometry_msgs::msg::Transform t;
  auto q = EulerToQuaternion(kPi, 0, 0.5 * kPi);
  hippo_common::convert::EigenToRos(q, t.rotation);
  return t;
}

geometry_msgs::msg::Transform NEDtoENU() {
  geometry_msgs::msg::Transform t;
  Eigen::Quaterniond q = EulerToQuaternion(kPi, 0.0, 0.5 * kPi).inverse();
  hippo_common::convert::EigenToRos(q, t.rotation);
  return t;
}

geometry_msgs::msg::Transform FLUtoFRD() {
  geometry_msgs::msg::Transform t;
  Eigen::Quaterniond q = EulerToQuaternion(kPi, 0.0, 0.0);
  hippo_common::convert::EigenToRos(q, t.rotation);
  return t;
}

geometry_msgs::msg::Transform FRDtoFLU() {
  geometry_msgs::msg::Transform t;
  Eigen::Quaterniond q = EulerToQuaternion(kPi, 0.0, 0.0).inverse();
  hippo_common::convert::EigenToRos(q, t.rotation);
  return t;
}

geometry_msgs::msg::Transform CameraLinkToCameraFrame() {
  geometry_msgs::msg::Transform t;
  Eigen::Quaternion q = EulerToQuaternion(-0.5 * kPi, 0.0, -0.5 * kPi);
  hippo_common::convert::EigenToRos(q, t.rotation);
  return t;
}

geometry_msgs::msg::Transform CameraFrameToCameraLink() {
  geometry_msgs::msg::Transform t;
  Eigen::Quaternion q =
      EulerToQuaternion(-0.5 * kPi, 0.0, -0.5 * kPi).inverse();
  hippo_common::convert::EigenToRos(q, t.rotation);
  return t;
}

geometry_msgs::msg::Quaternion RotateByQuaternion(
    const geometry_msgs::msg::Quaternion &_orientation,
    const geometry_msgs::msg::Quaternion &_rotation) {
  Eigen::Quaterniond q_orig{_orientation.w, _orientation.x, _orientation.y,
                            _orientation.z};
  Eigen::Quaterniond q_rot{_rotation.w, _rotation.x, _rotation.y, _rotation.z};
  geometry_msgs::msg::Quaternion q_new;
  hippo_common::convert::EigenToRos(q_orig * q_rot, q_new);
  return q_new;
}

geometry_msgs::msg::Pose PoseRosToPx4(const geometry_msgs::msg::Pose &_pose) {
  geometry_msgs::msg::Pose out;
  out.position.x = _pose.position.y;
  out.position.y = _pose.position.x;
  out.position.z = -_pose.position.z;
  Eigen::Quaterniond q;
  convert::RosToEigen(_pose.orientation, q);
  q = q_enu_ned * (q * q_flu_frd);
  convert::EigenToRos(q, out.orientation);
  return out;
}
geometry_msgs::msg::Pose PosePx4ToRos(const geometry_msgs::msg::Pose &_pose) {
  geometry_msgs::msg::Pose out;
  out.position.x = _pose.position.y;
  out.position.y = _pose.position.x;
  out.position.z = -_pose.position.z;
  Eigen::Quaterniond q;
  convert::RosToEigen(_pose.orientation, q);
  q = (q_enu_ned * q) * q_flu_frd;
  convert::EigenToRos(q, out.orientation);
  return out;
}

namespace frame_id {}  // namespace frame_id
}  // namespace tf2_utils
}  // namespace hippo_common
