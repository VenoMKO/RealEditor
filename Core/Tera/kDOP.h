#pragma once

#include "FStream.h"

#define NUM_PLANES	3
#define MAX_TRIS_PER_LEAF 5

struct FkDOP {
  float Min[NUM_PLANES];
  float Max[NUM_PLANES];

  FkDOP()
  {
    for (int32 idx = 0; idx < NUM_PLANES; ++idx)
    {
      Min[idx] = 0;
      Max[idx] = 0;
    }
  }

  friend FStream& operator<<(FStream& s, FkDOP& kDOP)
  {
    for (int32 nIndex = 0; nIndex < NUM_PLANES; nIndex++)
    {
      s << kDOP.Min[nIndex];
    }
    for (int32 nIndex = 0; nIndex < NUM_PLANES; nIndex++)
    {
      s << kDOP.Max[nIndex];
    }
    return s;
  }
};

struct FkDOPCompact {
  uint8 Min[NUM_PLANES];
  uint8 Max[NUM_PLANES];

  FkDOPCompact()
  {
    for (int32 idx = 0; idx < NUM_PLANES; ++idx)
    {
      Min[idx] = 0;
      Max[idx] = 0;
    }
  }

  friend FStream& operator<<(FStream& s, FkDOPCompact& kDOP)
  {
    for (int32 nIndex = 0; nIndex < NUM_PLANES; nIndex++)
    {
      s << kDOP.Min[nIndex];
    }
    for (int32 nIndex = 0; nIndex < NUM_PLANES; nIndex++)
    {
      s << kDOP.Max[nIndex];
    }
    return s;
  }
};

struct FkDOPNode {
  FkDOP BoundingVolume;
  bool IsLeaf = false;

  union
  {
    struct
    {
      uint16 LeftNode;
      uint16 RightNode;
    } n;
    struct
    {
      uint16 NumTriangles;
      uint16 StartIndex;
    } t;
  };

  FkDOPNode()
  {
    n.LeftNode = 0xFFFF;
    n.RightNode = 0xFFFF;
  }

  friend FStream& operator<<(FStream& s, FkDOPNode& n)
  {
    s << n.BoundingVolume << n.IsLeaf;
    s << n.n.LeftNode << n.n.RightNode;
    return s;
  }
};

struct FkDOPNodeCompact {
  FkDOPCompact BoundingVolume;

  friend FStream& operator<<(FStream& s, FkDOPNodeCompact& n)
  {
    return s << n.BoundingVolume;
  }
};

struct FkDOPCollisionTriangle {
  uint16 Vertex1 = 0;
  uint16 Vertex2 = 0;
  uint16 Vertex3 = 0;
  uint16 MaterialIndex = 0;

  friend FStream& operator<<(FStream& s, FkDOPCollisionTriangle& t)
  {
    return s << t.Vertex1 << t.Vertex2 << t.Vertex3 << t.MaterialIndex;
  }
};

struct FkDOPTreeCompact {
  uint32 NodesElementSize = sizeof(FkDOPNodeCompact);
  std::vector<FkDOPNodeCompact> Nodes;
  uint32 TrianglesElementSize = sizeof(FkDOPCollisionTriangle);
  std::vector<FkDOPCollisionTriangle> Triangles;
  FkDOP RootBounds;

  friend FStream& operator<<(FStream& s, FkDOPTreeCompact& t)
  {
    s << t.RootBounds;

    // Nodes
    
    s << t.NodesElementSize;
    DBreakIf(t.NodesElementSize != sizeof(FkDOPNodeCompact));
    int32 cnt = (int32)t.Nodes.size();
    s << cnt;
    if (s.IsReading())
    {
      t.Nodes.resize(cnt);
    }
    for (int32 idx = 0; idx < cnt; ++idx)
    {
      s << t.Nodes[idx];
    }

    // Triangles

    s << t.TrianglesElementSize;
    DBreakIf(t.TrianglesElementSize != sizeof(FkDOPCollisionTriangle));
    cnt = (int32)t.Triangles.size();
    s << cnt;
    if (s.IsReading())
    {
      t.Triangles.resize(cnt);
    }
    for (int32 idx = 0; idx < cnt; ++idx)
    {
      s << t.Triangles[idx];
    }
    return s;
  }
};

struct FkDOPTree {
  uint32 NodesElementSize = sizeof(FkDOPNode);
  std::vector<FkDOPNode> Nodes;
  uint32 TrianglesElementSize = sizeof(FkDOPCollisionTriangle);
  std::vector<FkDOPCollisionTriangle> Triangles;

  friend FStream& operator<<(FStream& s, FkDOPTree& t)
  {
    // Nodes

    s << t.NodesElementSize;
    DBreakIf(t.NodesElementSize != sizeof(FkDOPNode));
    int32 cnt = (int32)t.Nodes.size();
    s << cnt;
    if (s.IsReading())
    {
      t.Nodes.resize(cnt);
    }
    for (int32 idx = 0; idx < cnt; ++idx)
    {
      s << t.Nodes[idx];
    }

    //Triangles

    s << t.TrianglesElementSize;
    DBreakIf(t.TrianglesElementSize != sizeof(FkDOPCollisionTriangle));
    cnt = (int32)t.Triangles.size();
    s << cnt;
    if (s.IsReading())
    {
      t.Triangles.resize(cnt);
    }
    for (int32 idx = 0; idx < cnt; ++idx)
    {
      s << t.Triangles[idx];
    }
    return s;
  }
};