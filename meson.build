project('rgt-devkit', 'cpp',
  default_options: [
    'cpp_std=c++20'
  ]
)

cpp = meson.get_compiler('cpp')

ssl_dep = dependency('openssl', required: true) 

poco_foundation = cpp.find_library('PocoFoundation', required: true)
poco_json = cpp.find_library('PocoJSON', required: true)
poco_jwt = cpp.find_library('PocoJWT', required: true)
poco_net = cpp.find_library('PocoNet', required: true)
poco_redis = cpp.find_library('PocoRedis', required: true)
poco_data = cpp.find_library('PocoData', required: true)
poco_data_postgresql = cpp.find_library('PocoDataPostgreSQL', required: false)
poco_util = cpp.find_library('PocoUtil', required: true)
aws_core = cpp.find_library('aws-cpp-sdk-core', required: true)
aws_s3 = cpp.find_library('aws-cpp-sdk-s3', required: true)
rabbitmq = cpp.find_library('rabbitmq', required: true)
simpleamqpclient = cpp.find_library('SimpleAmqpClient', required: true)

inc = include_directories('lib')
sources = files(
  'lib/RGT/Devkit/General.cpp',
  'lib/RGT/Devkit/HTTPRequestHandler.cpp',
  'lib/RGT/Devkit/Subsystems/RedisSubsystem.cpp',
  'lib/RGT/Devkit/Subsystems/PsqlSubsystem.cpp',
  'lib/RGT/Devkit/Subsystems/S3Subsystem.cpp',
  'lib/RGT/Devkit/Subsystems/RabbitMQSubsystem.cpp',
  'lib/RGT/Devkit/Connections/PostgreSQL.cpp',
  'lib/RGT/Devkit/Connections/RabbitMQ.cpp',
  'lib/RGT/Devkit/Connections/Redis.cpp',
  'lib/RGT/Devkit/Connections/S3.cpp',
)

all_deps = [
  ssl_dep,
  poco_foundation,
  poco_json,
  poco_jwt,
  poco_net,
  poco_redis,
  poco_data,
  poco_data_postgresql,
  poco_util,
  aws_core,
  aws_s3,
  rabbitmq,
  simpleamqpclient
]

devkit_lib = shared_library('devkit', sources,
  include_directories: inc,
  dependencies: all_deps,
  cpp_args: ['-DENABLE_TRACING=0']
)

devkit_dep = declare_dependency(
  include_directories: inc,
  link_with: devkit_lib,
  dependencies: all_deps
)
