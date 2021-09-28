#ifndef __VISUAL_H
#define __VISUAL_H

#include <string>
#include <memory>

#include <CGRA/CGRA.h>
#include <CGRA/Mapping.h>

void genCGRAVisual(std::string exe_path, std::shared_ptr<CGRA> cgra, int II);
void genMappingVisual(std::string exe_path, const Mapping & mapping);

#endif

