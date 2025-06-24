#pragma once

#include <vcclr.h>
#include "iwgfx_extern.h"
#include <vector>

namespace PESMLIB
{
	__nogc class ShellMapCallback : public IwPolygonOutputCallback
	{
	public:
		ShellMapCallback(XML::XmlNode* XmlShellsNode);
		virtual ~ShellMapCallback(void);

		XML::XmlDocumentFragment* GetDocFragment();
		IwStatus OutputMesh(IwTArray<ULONG>& , IwTArray<ULONG> & ,
			IwTArray<IwVector3d> &, IwTArray<IwVector3d>&, IwTArray<IwPoint2d>&, 
			IwSurface *, IwFace*);
	protected:
		gcroot<XML::XmlDocument*> m_XmlDoc;
		gcroot<XML::XmlElement*> m_XmlShellPtsElem;
		gcroot<XML::XmlElement*> m_XmlShellFacesElem;
	};
}