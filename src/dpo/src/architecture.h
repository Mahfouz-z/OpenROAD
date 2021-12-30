///////////////////////////////////////////////////////////////////////////////
// BSD 3-Clause License
//
// Copyright (c) 2021, Andrew Kennings
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma once

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <iostream>
#include <limits>
#include <map>
#include <vector>
#include "orientation.h"
#include "rectangle.h"
#include "symmetry.h"

namespace dpo {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class RoutingParams;
class Architecture;
class Network;
class Node;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class Architecture {
  // This class represents information about the layout area. 

 public:
  class Row;
  class Spacing;
  class Region;

 public:
  Architecture();
  virtual ~Architecture();

  std::vector<Architecture::Row*>& getRows(void) { return m_rows; }
  int getNumRows(void) const { return m_rows.size(); }
  Architecture::Row* getRow(int r) const { return m_rows[r]; }
  Architecture::Row* createAndAddRow(void);

  std::vector<Architecture::Region*>& getRegions(void) { return m_regions; }
  int getNumRegions(void) const { return m_regions.size(); }
  Architecture::Region* getRegion(int r) const { return m_regions[r]; }
  Architecture::Region* createAndAddRegion(void);

  bool isSingleHeightCell(Node* ndi) const;
  bool isMultiHeightCell(Node* ndi) const;

  int getCellHeightInRows(Node* ndi) const;

  bool postProcess( Network* network );
  double compute_overlap(double xmin1, double xmax1, double ymin1, double ymax1,
                         double xmin2, double xmax2, double ymin2,
                         double ymax2);
  int find_closest_row(double y);

  void clear_edge_type();
  void init_edge_type();
  int add_edge_type(const char* name);

  double getMinX(void) const { return m_xmin; }
  double getMaxX(void) const { return m_xmax; }
  double getMinY(void) const { return m_ymin; }
  double getMaxY(void) const { return m_ymax; }

  void setMinX(double xmin) { m_xmin = xmin; }
  void setMaxX(double xmax) { m_xmax = xmax; }
  void setMinY(double ymin) { m_ymin = ymin; }
  void setMaxY(double ymax) { m_ymax = ymax; }

  double getWidth(void) const { return m_xmax - m_xmin; }
  double getHeight(void) const { return m_ymax - m_ymin; }

  bool power_compatible(Node* ndi, Row* row, bool& flip);

  // Well, sometimes I see padding used and other times I see
  // spacing tables used.  I don't think both should be used.
  // We need to give priority to one over the other in the
  // event that both are set to true!

  // Using tables...
  void setUseSpacingTable(bool val = true) { m_useSpacingTable = val; }
  bool getUseSpacingTable(void) const { return m_useSpacingTable; }
  void clearSpacingTable(void);
  double getCellSpacingUsingTable(int firstEdge, int secondEdge);
  void addCellSpacingUsingTable(int firstEdge, int secondEdge, double sep);
  std::vector<Architecture::Spacing*>& getCellSpacings(void) { 
    return m_cellSpacings;
  }
  std::vector<std::pair<char*,int> >& getEdgeTypes(void) { 
    return m_edgeTypes;
  }

  // Using padding...
  void setUsePadding(bool val = true) { m_usePadding = val; }
  bool getUsePadding(void) const { return m_usePadding; }
  void addCellPadding(Node* ndi, double leftPadding, double rightPadding);
  bool getCellPadding(Node* ndi, double& leftPadding, double& rightPadding);

  double getCellSpacing(Node* leftNode, Node* rightNode);

 protected:
  // Boundary around rows.
  double m_xmin;
  double m_xmax;
  double m_ymin;
  double m_ymax;

  // Rows...
  std::vector<Row*> m_rows;

  // Regions...
  std::vector<Region*> m_regions;

  // Spacing tables...
  bool m_useSpacingTable;
  std::vector<std::pair<char*, int> > m_edgeTypes;
  std::vector<Spacing*> m_cellSpacings;

  // Padding...
  bool m_usePadding;
  std::map<int, std::pair<double, double> >
      m_cellPaddings;  // Padding to left,right.
};

class Architecture::Spacing {
 public:
  Spacing(int i1, int i2, double sep);
  virtual ~Spacing();

  int getFirstEdge(void) const { return m_i1; }
  int getSecondEdge(void) const { return m_i2; }
  double getSeparation(void) const { return m_sep; }

 protected:
  int m_i1;
  int m_i2;
  double m_sep;
};

const unsigned RowPower_UNK = 0x00000001;
const unsigned RowPower_VDD = 0x00000002;
const unsigned RowPower_VSS = 0x00000004;

class Architecture::Row {
 public:
  Row();
  virtual ~Row();

  void setBottom(double bottom) { m_rowLoc = bottom; }
  void setHeight(double height) { m_rowHeight = height; }
  void setSiteSpacing(double spacing) { m_siteSpacing = spacing; }
  void setSiteWidth(double width) { m_siteWidth = width; }
  void setNumSites(int nsites) { m_numSites = nsites; }

  inline double getBottom(void) const { return m_rowLoc; }
  inline double getTop(void) const { return m_rowLoc+m_rowHeight; }
  inline double getCenterY(void) const { return m_rowLoc+0.5*m_rowHeight; }
  inline double getLeft(void) const { return m_subRowOrigin; }
  inline double getRight(void) const { 
    return m_subRowOrigin+m_numSites*m_siteSpacing; // ? add m_siteWidth
  }
  inline double getHeight(void) const { return m_rowHeight; }
  inline double getSiteWidth(void) const { return m_siteWidth; }
  inline double getSiteSpacing(void) const { return m_siteSpacing; }
  inline int getNumSites(void) const { return m_numSites; }

 protected:
  double m_rowLoc;          // Y-location of the row.
  double m_rowHeight;       // Height of the row.
  int m_numSites;           // Number of sites...  Ending X location (xmax) is =
                            // m_subRowOrigin + m_numSites * m_siteSpacing;
  double m_siteWidth;       // Width of sites in the row.

 public:
  // XXX: Ignores site orientation and symmetry right now...
  double m_subRowOrigin;    // Starting X location (xmin) of the row.
  unsigned m_siteOrient;    // Orientation of sites in the row.
  unsigned m_siteSymmetry;  // Symmetry of sites in the row.  Symmetry allows
                            // for certain orientations...
  double m_siteSpacing;     // Spacing between sites in the row. XXX: Likely
                            // assumed to be the same as the width...
  int m_id;  // Every row  needs an id...  Filled in after sorting.

  // The following is to try and monitor voltages at the top and bottom of the
  // row. It is from the ICCAD 2017 contest.  I'm not sure it is the most
  // general, but I think it will work for the contest.
  int m_powerTop;
  int m_powerBot;

  struct compare_row {
    inline bool operator()(Architecture::Row*& s, double i) const {
      return s->getBottom() < i;
    }
    inline bool operator()(double i, Architecture::Row*& s) const {
      return i < s->getBottom();
    }
  };
};

struct SortRow {
  inline bool operator()(Architecture::Row* p, Architecture::Row* q) const {
    return p->getBottom() < q->getBottom();
  }
};

class Architecture::Region {
 public:
  Region();
  virtual ~Region();

 public:
  int m_id;  // Artificial ID of the region.

  // Box around all sub-rectangles.
  double m_xmin;
  double m_ymin;
  double m_xmax;
  double m_ymax;

  // Rectangles forming the rectilinear region.
  std::vector<Rectangle> m_rects;
};

}  // namespace dpo