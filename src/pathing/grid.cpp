#include "grid.h"

namespace Aeolus
{
	Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic> Grid::GetGrid()
	{
		return m_grid;
	}
}