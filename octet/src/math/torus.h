////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Torus in 3d space
//
// By Mohammad Ali
namespace octet { namespace math {
		// Torus: Donut / Ring Shape
		class torus {
			vec3 center;
			float in_radius;
			float out_radius;
		public:
			torus(vec3_in center_ = vec3(0, 0, 0), float iradius_ = 0.1, float oradius_ = 2) {
				center = center_;
				in_radius = iradius_;
				out_radius = oradius_;
			}

			vec3_ret get_center() const {
				return center;
			}

			float get_outradius() const {
				return out_radius;
			}

			float get_inradius() const {
				return in_radius;
			}

			vec3 get_min() const {
				return center - (out_radius + in_radius);
			}

			vec3 get_max() const {
				return center + (out_radius + in_radius);
			}

			aabb get_aabb() const {
				return aabb(center, vec3((out_radius + in_radius)));
			}

			// get transformed sphere assuming mat is orthonormal.
			torus get_transform(const mat4t &mat) const {
				return torus(get_center(), get_inradius(), get_outradius());
			}

			const char *toString(char *dest, size_t len) const {
				static char tmp[64];
				snprintf(dest, len, "[%s, %f, %f]", center.toString(tmp, sizeof(tmp)), in_radius, out_radius);
				return dest;
			}

			template <class sink_t> void get_geometry(sink_t &sink, int steps) const {

				sink.reserve(steps * 4, steps * 6 + (steps - 2) * 6);  //Why reserving size?


				float k = (3.14159265f * 2) / steps;
				float ku = 1.0f / steps;
				for (unsigned i = 0; i != steps; ++i) {
					for (unsigned j = 0; j != steps; ++j) {
						float c = cosf(i * k);
						float s = sinf(i * k);
						float c2 = cosf(j * k);
						float s2 = sinf(j * k);
						//position , normal , uvw
						sink.add_vertex(center + out_radius + in_radius + vec3(c*in_radius, s*in_radius, -s2*out_radius), vec3(c, s, 0), vec3(i * ku, 0, 0));
						sink.add_vertex(center - out_radius + in_radius + vec3(c*in_radius, s*in_radius, s2*out_radius), vec3(c, s, 0), vec3(i * ku, 1, 0));
						sink.add_vertex(center + out_radius + in_radius + vec3(c*in_radius, s*in_radius, -s2*out_radius), vec3(0, 0, -1), vec3(c, s, -1));
						sink.add_vertex(center - out_radius - in_radius + vec3(c*in_radius, s*in_radius, s2*out_radius), vec3(0, 0, 1), vec3(c, s, 1));
					}
				}

				// sides
				for (unsigned i = 0; i != steps; ++i) {
					unsigned i0 = i * 4;
					unsigned i1 = (i + 1 == steps) ? 0 * 4 : (i + 1) * 4;
					//x y z
					sink.add_triangle(i0 + 0, i1 + 0, i1 + 1);
					sink.add_triangle(i0 + 0, i1 + 1, i0 + 1);
				}

				//// bottom
				//for (unsigned i = 0; i != steps - 2; ++i) {
				//	sink.add_triangle(2, i * 4 + 6, i * 4 + 10);
				//}

				//// top
				//for (unsigned i = 0; i != steps - 2; ++i) {
				//	sink.add_triangle(3, i * 4 + 7, i * 4 + 11);
				//}
			}
		};
	}
}


