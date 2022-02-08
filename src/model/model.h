#ifndef MODEL_H_
#define MODEL_H_

#include <system_error>
#include <vector>

class model_t {
public:
	struct Face {
		int v_idx[3];
		int tex_idx[3];
		int n_idx[3];
	};

	/** Load a model.
	 * Model data may be loaded to memory by bootloader or loaded from a file.
	 * This class automatically handles OS/bare metal differences and constructs
	 * an object.
	 *
	 * This class doesn't throw any exception. To check if data is loaded call
	 * is_loaded() method.
	 *
	 * @param model_filename: Identifier to load .obj file from
	 * @param texture_filename: Identifier to load .tga texture from
	 */
	model_t(const char *model_filename, const char *texture_filename) noexcept;

	bool is_loaded(void) const noexcept;

//private:
	std::errc load_texture(const char *filename) noexcept;

	bool is_loaded_;
	std::vector<Face> faces_;
	std::vector<std::vector<float>> vertices_;
	std::vector<std::vector<float>> normals_;
	std::vector<std::vector<float>> texture_;
	std::vector<uint32_t> texture_image_; /* colors in RGB888 format */
	size_t texture_width_;
	size_t texture_height_;
};

#endif /* MODEL_H_ */
