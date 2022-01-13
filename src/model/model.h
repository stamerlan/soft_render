#ifndef MODEL_H_
#define MODEL_H_

#include <vector>

class Model {
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
	 * @param name: model name
	 */
	Model(const char *name) noexcept;

	bool is_loaded(void) const noexcept;

//private:
	bool is_loaded_;
	std::vector<Face> faces_;
	std::vector<std::vector<float>> vertices_;
	std::vector<std::vector<float>> normals_;
};

#endif /* MODEL_H_ */
