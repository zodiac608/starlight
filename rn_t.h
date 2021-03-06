/******************************************************************************
file: rn_t.h
desc: 
 *****************************************************************************/
#pragma once

#include <GL/glew.h>		//extension enumerator (has to be here for link)
#include <GL/gl.h>			//gl types
#include <vector>			//stl vector
#include <string>

#define BG_COLOR 0.0f, 0.2f, 0.3f, 1.0f
#define DEPTH_CLEAR 1.0f
#define DR_NEAR 0.0f
#define DR_FAR 1.0f
#define VTX_SHADER_DIR "shader/vertex/"
#define FRAG_SHADER_DIR "shader/fragment/"
#define MAX_VAOS 16

#define ARRAY_COUNT( array ) (sizeof( array ) / (sizeof( array[0] ) * (sizeof( array ) != sizeof(void*) || sizeof( array[0] ) <= sizeof(void*))))

using std::vector;
using std::string;

class rn_t
{
	public:
		/*functions*/
		rn_t( void );
		~rn_t( void ) {};

		int start( void );
		int draw( void );
		int shutdown( void );
		int resize_gl( int w, int h );

	private:
		/*functions*/
		int load_all_shaders( void );
		int compile_shader( GLuint* sh_num,
				GLenum sh_type,
				char* sh_file,
				string sh_file_name );
		int create_sh_program( void );
		int delete_all_shaders( void );
		int init_vtx_b( void );
		int init_vaos( void );
		int set_gl_state( void );
		int initialize_shaders( void );
		
		/*attribs*/
		vector<GLuint> r_shader_list;
		GLuint r_sh_program;
		GLuint r_pos_bobj;
		GLuint r_index_bobj;
		GLuint r_vao_ar[MAX_VAOS];
		int r_used_vaos;

		GLfloat ps_matrix[16];		//perspective matrix
		GLfloat f_scale;			//frustrum scale
		GLfloat z_near;				//z near
		GLfloat z_far;				//z far
};


