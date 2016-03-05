////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Mesh smooth modifier. Work in progress.
//


//Modified to be a torus 
//Could not make custom class compile
namespace octet { namespace scene {
  /// Box mesh. Generate triangles for an AABB.
  class mesh_cylinder : public mesh {
    //zcylinder cylinder;
	torus tor;
    mat4t transform;
    int steps;

    void init(torus_in tors, mat4t_in transform, int steps) {
      this->transform = transform;
      this->tor = tors;
      this->steps = steps;
      set_default_attributes();
      set_aabb(tor.get_aabb().get_transform(transform));
      update();
    }

  public:
    RESOURCE_META(mesh_cylinder)

    /// Construct cylinder mesh from shape
    mesh_cylinder(torus_in tor=torus(), mat4t_in transform = mat4t(), int steps=32) {
		init(tor, transform, steps);
	}

    void set_size(torus_in tor=torus(), mat4t_in transform = mat4t(), int steps=32) {
      init(tor, transform, steps);
    }

    /// Generate mesh from parameters.
    virtual void update() {
      mesh::set_shape<torus, mesh::vertex>(tor, transform, steps);
    }

    /// Serialise the box
    void visit(visitor &v) {
      mesh::visit(v);
      if (v.is_reader()) {
        update();
      }
    }

    #ifdef OCTET_BULLET
      /// Get a bullet shape object for this mesh
      btCollisionShape *get_bullet_shape() {
        return new btCylinderShapeZ(btVector3((tor.get_inradius() * 2) + tor.get_outradius(), (tor.get_inradius() * 2) + tor.get_outradius(), tor.get_inradius()));
      }

      /// Get a static bullet shape object for this mesh
      btCollisionShape *get_static_bullet_shape() {
        return new btCylinderShapeZ(btVector3((tor.get_inradius() * 2) + tor.get_outradius(), (tor.get_inradius() * 2) + tor.get_outradius(), tor.get_inradius()));
      }
    #endif
  };
}}

