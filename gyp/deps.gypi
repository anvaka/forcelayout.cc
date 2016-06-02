{
# These dependencies are shared between test and static library
  'dependencies': [
    "<!(node -e \"console.log(require.resolve('nangraph.cpp/gyp/nangraph.cc.gyp') + ':*')\")",
    "<!(node -e \"console.log(require.resolve('quadtree.cc/gyp/quadtree.cc.gyp') + ':*')\")",
  ],
}
