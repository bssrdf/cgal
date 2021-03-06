#ifndef SCENE_NEF_POLYHEDRON_ITEM_H
#define SCENE_NEF_POLYHEDRON_ITEM_H
#include  <CGAL/Three/Scene_item.h>
#include "Scene_nef_polyhedron_item_config.h"
#include "Nef_type_fwd.h"
#include <iostream>
#include <queue>
class Scene_polyhedron_item;
struct Scene_nef_polyhedron_item_priv;
class SCENE_NEF_POLYHEDRON_ITEM_EXPORT Scene_nef_polyhedron_item
 : public CGAL::Three::Scene_item
{
  Q_OBJECT
public:
  Scene_nef_polyhedron_item();
//   Scene_nef_polyhedron_item(const Scene_nef_polyhedron_item&);
  Scene_nef_polyhedron_item(const Nef_polyhedron& p);
  Scene_nef_polyhedron_item(Nef_polyhedron* const p);
  ~Scene_nef_polyhedron_item();

  Scene_nef_polyhedron_item* clone() const;
  bool load_from_off(std::istream& in);
  bool load(std::istream& in);
  bool save(std::ostream& in) const;

  QFont font() const;
  QString toolTip() const;

  virtual void invalidateOpenGLBuffers();
  virtual void selection_changed(bool);
  // Indicate if rendering mode is supported
  virtual bool supportsRenderingMode(RenderingMode m) const { return m != Gouraud && m!=Splatting && m!=ShadedPoints; } // CHECK THIS!
  // OpenGL drawing in a display list
  void direct_draw() const;

  virtual void draw(CGAL::Three::Viewer_interface*) const;
  virtual void drawEdges() const {}
  virtual void drawEdges(CGAL::Three::Viewer_interface* viewer) const;
  virtual void drawPoints(CGAL::Three::Viewer_interface*) const;
  // Wireframe OpenGL drawing

  bool isFinite() const { return true; }
  bool isEmpty() const;
  void compute_bbox() const;

  Nef_polyhedron* nef_polyhedron();

  bool is_simple() const;
  bool is_Triangle;
  // conversion operations
  static Scene_nef_polyhedron_item* from_polyhedron(Scene_polyhedron_item*);
  Scene_polyhedron_item* convert_to_polyhedron() const;

  // Nef boolean operations
  Scene_nef_polyhedron_item&
  operator+=(const Scene_nef_polyhedron_item&); // union

  Scene_nef_polyhedron_item&
  operator*=(const Scene_nef_polyhedron_item&); // intersection

  Scene_nef_polyhedron_item&
  operator-=(const Scene_nef_polyhedron_item&); // difference

  static Scene_nef_polyhedron_item*
  sum(const Scene_nef_polyhedron_item&, 
      const Scene_nef_polyhedron_item&);

  void convex_decomposition(std::list< Scene_polyhedron_item*>&);
protected:
  friend struct Scene_nef_polyhedron_item_priv;
  Scene_nef_polyhedron_item_priv* d;
}; // end class Scene_nef_polyhedron_item

#endif // SCENE_NEF_POLYHEDRON_ITEM_H
