"""
This launch file starts all nodes that are not required to run directly on the
board computer to perform the lemniscate demo.

Example:
    ros2 launch hippo_common top_lemniscate_demo_offboard.launch.py \
    vehicle_name:=uuv02 \
    use_sim_time:=false \
    use_apriltags:=true \
    vehicle_type:=hippocampus
"""

from launch_ros.actions import Node

from hippo_common.launch_helper import (
    LaunchArgsDict,
    config_file_path,
    declare_vehicle_name_and_sim_time,
    declare_vehicle_type,
    launch_file_source,
    require_hippocampus_vehicle_type,
)
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    IncludeLaunchDescription,
)
from launch.conditions import IfCondition
from launch.substitutions import (
    EqualsSubstitution,
    LaunchConfiguration,
    NotSubstitution,
)


def declare_launch_args(launch_description: LaunchDescription):
    declare_vehicle_name_and_sim_time(launch_description)
    declare_vehicle_type(launch_description)

    action = require_hippocampus_vehicle_type()
    launch_description.add_action(action)

    action = DeclareLaunchArgument('use_apriltags')
    launch_description.add_action(action)

    pkg = 'hippo_common'
    default = config_file_path(pkg, 'transformations_hippo_default.yaml')
    vehicle_type = LaunchConfiguration('vehicle_type')
    action = DeclareLaunchArgument(
        name='tf_vehicle_config_file',
        description='tf config file',
        default_value=default,
        condition=IfCondition(EqualsSubstitution(vehicle_type, 'hippocampus')),
    )
    launch_description.add_action(action)
    default = config_file_path(pkg, 'transformations_bluerov_default.yaml')
    action = DeclareLaunchArgument(
        name='tf_vehicle_config_file',
        description='tf config file',
        default_value=default,
        condition=IfCondition(EqualsSubstitution(vehicle_type, 'bluerov')),
    )
    launch_description.add_action(action)


def include_path_follower():
    pkg = 'hippo_control'
    file = 'top_path_following_intra_process.launch.py'
    source = launch_file_source(pkg, file)
    args = LaunchArgsDict()
    args.add_vehicle_name_and_sim_time()
    return IncludeLaunchDescription(source, launch_arguments=args.items())


def include_visual_localization(condition):
    args = LaunchArgsDict()
    args.add_vehicle_name_and_sim_time()
    pkg = 'visual_localization'
    source = launch_file_source(pkg, 'top_localization.launch.py')
    return IncludeLaunchDescription(
        source,
        launch_arguments=args.items(),
        condition=IfCondition(condition),
    )


def include_qualisys_localization(condition):
    args = LaunchArgsDict()
    args.add_vehicle_name_and_sim_time()
    pkg = 'qualisys_bridge'
    source = launch_file_source(pkg, 'qualisys_bridge.launch.py')
    return IncludeLaunchDescription(
        source, launch_arguments=args.items(), condition=IfCondition(condition)
    )


def add_tf_publisher_vehicle_node():
    args = LaunchArgsDict()
    args.add_vehicle_name_and_sim_time()
    return Node(
        package='hippo_common',
        namespace=LaunchConfiguration('vehicle_name'),
        executable='tf_publisher_vehicle_node',
        parameters=[args, LaunchConfiguration('tf_vehicle_config_file')],
    )


def add_px4_bridge_node():
    args = LaunchArgsDict()
    args.add_vehicle_name_and_sim_time()
    return Node(
        package='visual_localization',
        executable='px4_bridge',
        namespace=LaunchConfiguration('vehicle_name'),
        name='px4_bridge',
        parameters=[args],
        output='screen',
        emulate_tty=True,
    )


def generate_launch_description():
    launch_description = LaunchDescription()
    declare_launch_args(launch_description=launch_description)
    actions = [
        include_path_follower(),
        add_tf_publisher_vehicle_node(),
        add_px4_bridge_node(),
        include_visual_localization(LaunchConfiguration('use_apriltags')),
        include_qualisys_localization(
            NotSubstitution(LaunchConfiguration('use_apriltags'))
        ),
    ]
    for action in actions:
        launch_description.add_action(action)
    return launch_description
