/**
 * Model class implementation to load a model from file and allocate its data
 * on heap using STL containers.
 */
#include "model.h"
#include <cassert>
#include <charconv>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

static void trim(std::string &s)
{
	/* left trim */
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c) {
			return !std::isspace(c);
		}));
	/* right trim */
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char c) {
			return !std::isspace(c);
		}).base(), s.end());
}

static std::vector<float> parse_coord(std::string_view str)
{
	std::vector<float> ret;

	for (auto p = str.data(); p < str.data() + str.size();) {
		/* skip leading spaces */
		for (; p < str.data() + str.size() && isspace(*p); p++);
		if (p >= str.data() + str.size())
			break;

		float f;
		auto conv_result = std::from_chars(p, str.data() + str.size(), f);
		if (conv_result.ec != std::errc())
			return {};
		ret.push_back(f);
		p = conv_result.ptr;
	}

	return ret;
}

/* TODO: add error handling in case input string is invalid or doesn't contain
 * normal/texture indices
 */
static struct Model::Face parse_faces(std::string_view str)
{
	struct Model::Face ret;

	size_t idx = 0;
	for (auto p = str.data(); p < str.data() + str.size(); idx++) {
		/* skip leading spaces */
		for (; p < str.data() + str.size() && isspace(*p); p++);
		if (p >= str.data() + str.size())
			return {};

		int f;
		auto conv_result = std::from_chars(p, str.data() + str.size(), f);
		if (conv_result.ec != std::errc())
			return {};
		ret.v_idx[idx] = f;
		p = conv_result.ptr;

		/* skip slash */
		if (p >= str.data() + str.size() || p[0] != '/')
			return {};
		p++;

		conv_result = std::from_chars(p, str.data() + str.size(), f);
		if (conv_result.ec != std::errc())
			return {};
		ret.tex_idx[idx] = f;
		p = conv_result.ptr;

		/* skip slash */
		if (p >= str.data() + str.size() || p[0] != '/')
			return {};
		p++;

		conv_result = std::from_chars(p, str.data() + str.size(), f);
		if (conv_result.ec != std::errc())
			return {};
		ret.n_idx[idx] = f;
		p = conv_result.ptr;
	}

	return ret;
}

/* TODO: malformed files */
std::errc Model::load_texture(const char *filename) noexcept
{
	std::ifstream file(filename, std::ifstream::binary);
	if (!file.is_open()) {
		std::cerr << "Failed to open " << std::quoted(filename) << "\n";
		return std::errc::no_such_file_or_directory;
	}

	struct {
		/* 0x00 */ uint8_t id_len;
		/* 0x01 */ uint8_t color_map_type;
		/* 0x02 */ uint8_t image_type;

		/* 0x03 */ uint8_t color_map_spec[5];

		/* 0x08 */ uint16_t x;
		/* 0x0A */ uint16_t y;
		/* 0x0C */ uint16_t width;
		/* 0x0E */ uint16_t height;
		/* 0x10 */ uint8_t  bpp;
		/* 0x11 */ uint8_t  img_desc;
	} tga_hdr;
	static_assert(sizeof(tga_hdr) == 18);

	/* TODO: endianness
	 * fields in TGA file are stored in little-endian format
	 */
	file.read((char *)&tga_hdr, sizeof(tga_hdr));

	/* Run-length encoded (RLE), True-color images supported only */
	if (tga_hdr.color_map_type != 0 || tga_hdr.image_type != 10)
		return std::errc::function_not_supported;
	if (tga_hdr.x || tga_hdr.y)
		return std::errc::function_not_supported;
	if (tga_hdr.bpp != 24)
		return std::errc::function_not_supported;
	if (tga_hdr.img_desc != 0)
		return std::errc::function_not_supported;

	texture_width_ = tga_hdr.width;
	texture_height_ = tga_hdr.height;
	texture_image_.resize(texture_width_ * texture_height_);

	file.seekg(tga_hdr.id_len, std::ios_base::cur); /* seek to pixel data */

	/* IMAGE TYPE 2: Image Data Field.
	 * 
	 * This field specifies (width) x (height) pixels. Each
	 * pixel specifies an RGB color value, which is stored as
	 * an integral number of bytes.
	 * The 2 byte entry is broken down as follows:
	 * ARRRRRGG GGGBBBBB, where each letter represents a bit.
	 * But, because of the lo-hi storage order, the first byte
	 * coming from the file will actually be GGGBBBBB, and the
	 * second will be ARRRRRGG. "A" represents an attribute bit.
	 * The 3 byte entry contains 1 byte each of blue, green,
	 * and red.
	 * The 4 byte entry contains 1 byte each of blue, green,
	 * red, and attribute. For faster speed (because of the
	 * hardware of the Targa board itself), Targa 24 images are
	 * sometimes stored as Targa 32 images.
	 *
	 *
	 * IMAGE TYPE 10: Image Data Field.
	 *
	 * This field specifies (width) x (height) pixels. The
	 * RGB color information for the pixels is stored in
	 * packets. There are two types of packets: Run-length
	 * encoded packets, and raw packets. Both have a 1-byte
	 * header, identifying the type of packet and specifying a
	 * count, followed by a variable-length body.
	 * The high-order bit of the header is "1" for the
	 * run length packet, and "0" for the raw packet.
	 *
	 * For the run-length packet, the header consists of:
	 *     __________________________________________________
	 *     | 1 bit |   7 bit repetition count minus 1.      |
	 *     |   ID  |   Since the maximum value of this      |
	 *     |       |   field is 127, the largest possible   |
	 *     |       |   run size would be 128.               |
	 *     |-------|----------------------------------------|
	 *     |   1   |  C     C     C     C     C     C    C  |
	 *     --------------------------------------------------
	 *
	 * For the raw packet, the header consists of:
	 *     __________________________________________________
	 *     | 1 bit |   7 bit number of pixels minus 1.      |
	 *     |   ID  |   Since the maximum value of this      |
	 *     |       |   field is 127, there can never be     |
	 *     |       |   more than 128 pixels per packet.     |
	 *     |-------|----------------------------------------|
	 *     |   0   |  N     N     N     N     N     N    N  |
	 *     --------------------------------------------------
	 *
	 * For the run length packet, the header is followed by
	 * a single color value, which is assumed to be repeated
	 * the number of times specified in the header. The
	 * packet may cross scan lines ( begin on one line and end
	 * on the next ).
	 * For the raw packet, the header is followed by
	 * the number of color values specified in the header.
	 * The color entries themselves are two bytes, three bytes,
	 * or four bytes ( for Targa 16, 24, and 32 ), and are
	 * broken down as follows:
	 * The 2 byte entry -
	 * ARRRRRGG GGGBBBBB, where each letter represents a bit.
	 * But, because of the lo-hi storage order, the first byte
	 * coming from the file will actually be GGGBBBBB, and the
	 * second will be ARRRRRGG. "A" represents an attribute bit.
	 * The 3 byte entry contains 1 byte each of blue, green,
	 * and red.
	 * The 4 byte entry contains 1 byte each of blue, green,
	 * red, and attribute. For faster speed (because of the
	 * hardware of the Targa board itself), Targa 24 image are
	 * sometimes stored as Targa 32 images.
	 */
	enum class packet_type {
		none,
		run_len,
		raw,
	} packet_type = packet_type::none;
	uint32_t color = 0;
	unsigned cnt;

	for (size_t y = 0; y < texture_width_; y++) {
		for (size_t x = 0; x < texture_height_; x++) {
			if (packet_type == packet_type::none) {
				uint8_t packet;
				file.read((char *)&packet, sizeof(packet));

				if (packet & 0x80) {
					packet_type = packet_type::run_len;
					file.read((char *)&color, 3); /* bpp == 24, TODO: enianness */
				} else {
					packet_type = packet_type::raw;
				}
				cnt = (packet & ~0x80) + 1;
			}

			if (packet_type == packet_type::raw) {
				file.read((char *)&color, 3); /* bpp == 24, TODO: enianness */
			}

			texture_image_[y * texture_width_ + x] = color;
			if (--cnt == 0)
				packet_type = packet_type::none;
		}
	}

	return {};
}

Model::Model(const char *model_filename, const char *texture_filename) noexcept
{
	is_loaded_ = false;

	std::ifstream file(model_filename);
	if (!file.is_open()) {
		std::cerr << "Failed to open " << std::quoted(model_filename) << "\n";
		return;
	}

	std::string str;

	while (std::getline(file, str)) {
		/* TODO: use string_view to avoid copy */
		trim(str);

		if (str[0] == '#' || str.empty()) {
			/* a comment or empty line */
			continue;
		}

		if (str.starts_with("v ")) {
			/* TODO: list of geometric vertices, with (x, y, z [,w])
			 * coordinates, w is optional and defaults to 1.0.
			 */
			auto coord = parse_coord({ str.begin() + 2, str.end() });
			if (coord.size() != 3) {
				std::cerr << "Unexpected vertex: " << std::quoted(str) << ". {";
				for (auto f : coord) {
					std::cerr << " " << f;
				}
				std::cerr << " }\n";
				return;
			}
			vertices_.push_back(coord);
			continue;
		}

		if (str.starts_with("vn ")) {
			/* TODO: List of vertex normals in (x,y,z) form; normals might not
			 * be unit vectors. */
			auto coord = parse_coord({ str.begin() + 2, str.end() });
			if (coord.size() != 3) {
				std::cerr << "Unexpected normal: " << std::quoted(str) << ". {";
				for (auto f : coord) {
					std::cerr << " " << f;
				}
				std::cerr << " }\n";
				return;
			}
			normals_.push_back(coord);

			continue;
		}

		if (str.starts_with("vt ")) {
			/* TODO: List of texture coordinates, in (u, [,v ,w]) coordinates,
			 * these will vary between 0 and 1. v, w are optional and default to
			 * 0. */
			auto coord = parse_coord({ str.begin() + 2, str.end() });
			if (coord.size() != 2 && coord.size() != 3) {
				std::cerr << "Unexpected texture: " << std::quoted(str) << ". {";
				for (auto f : coord) {
					std::cerr << " " << f;
				}
				std::cerr << " }\n";
				return;
			}
			if (coord.size() == 3) {
				/* TODO: handle w */
				coord.pop_back();
			}
			texture_.push_back(coord);
			continue;
		}

		if (str.starts_with("f ")) {
			/* TODO: Polygonal face element (see below)
			 * f 1 2 3             # Vertex indices
			 * f 3/1 4/2 5/3       # Vertex texture coordinate indices
			 * f 6/4/1 3/5/3 7/6/5 # Vertex normal indices
			 * f 7//1 8//2 9//3    # Vertex normal indices w/o texture coordinates
			 */
			auto faces = parse_faces({ str.begin() + 2, str.end() });
			/* TODO: check if faces are valid */
			faces_.push_back(faces);
			continue;
		}

		if (str.starts_with("g ") || (str.size() == 1 && str[0] == 'g')) {
			/* skip group name */
			continue;
		}

		if (str.starts_with("s ")) {
			/* skip smooth shading */
			continue;
		}

		if (str.starts_with("mtllib")) {
			/* skip material lib */
			continue;
		}

		if (str.starts_with("usemtl ")) {
			/* skip material name */
			continue;
		}

		/* TODO: Log warning */
		assert(false);
	}

	std::cerr << "Model " << std::quoted(model_filename) << " loaded. Faces: " <<
		faces_.size() << ", Vertices: " << vertices_.size() << 
		", Normals: " << normals_.size() << "\n";

	if (load_texture(texture_filename) != std::errc()) {
		std::cerr << "Texture loading failed\n";
		return;
	}

	is_loaded_ = true;
}

bool Model::is_loaded(void) const noexcept
{
	return is_loaded_;
}
