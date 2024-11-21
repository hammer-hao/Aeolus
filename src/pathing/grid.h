#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <sc2api/sc2_interfaces.h>
#include <sc2api/sc2_map_info.h>

#pragma warning(disable: 4127)
#pragma warning(disable: 4244)
#include <Eigen/Dense>

namespace Aeolus
{
	class Grid
	{
	public:

        // Default constructor
        Grid()
            : m_width(0), m_height(0), m_grid(0, 0) // Initialize with empty dimensions
        {
        }

        Grid(const Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic>& matrix)
            : m_width(matrix.cols()), m_height(matrix.rows()), m_grid(matrix)
        {
            std::cout << "Grid initialized from Eigen::Matrix with dimensions: "
                << m_height << "x" << m_width << std::endl;
        }

		Grid(::sc2::ImageData image_data)
            : m_width(image_data.width), m_height(image_data.height),
            m_grid(m_height, m_width)
		{

            if (image_data.bits_per_pixel == 8)
            {
                // Each byte represents a single pixel value (0-255)
                if (image_data.data.size() != static_cast<size_t>(m_width * m_height)) {
                    throw std::invalid_argument("Grid: ImageData data size does not match expected dimensions for 8 bits per pixel.");
                }

                // Fill the grid directly from the data
                for (int y = 0; y < m_height; ++y) {
                    for (int x = 0; x < m_width; ++x) {
                        m_grid(y, x) = static_cast<int>(image_data.data[y * m_width + x]);
                    }
                }
            }
            else if (image_data.bits_per_pixel == 1)
            {
                size_t expected_size = (m_width * m_height + 7) / 8; // Round up to account for any extra bits
                if (image_data.data.size() != expected_size) {
                    std::cout << m_width << " " << m_height << std::endl;
                    std::cout << "Incorrect size!" << image_data.data.size() << " " << expected_size << std::endl;
                    throw std::invalid_argument("Grid: ImageData data size does not match expected packed dimensions.");
                }

                // Fill the grid data
                for (int y = 0; y < m_height; ++y) {
                    for (int x = 0; x < m_width; ++x) {
                        size_t byte_index = (y * m_width + x) / 8;
                        size_t bit_index = 7 - ((y * m_width + x) % 8); // 7 - ... to read bits from MSB to LSB

                        // Extract the bit value
                        unsigned char byte = image_data.data[byte_index];
                        unsigned char bit = (byte >> bit_index) & 1;

                        m_grid(y, x) = static_cast<int>(bit);
                    }
                }
            }
		}

        Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> GetGrid();

	private:
		int m_width;
		int m_height;
        Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> m_grid;
	};
}