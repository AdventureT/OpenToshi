/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <ContentTools/Common/Filters/FilterTutorial/hctFilterTutorial.h>

#include <ContentTools/Common/Filters/FilterTutorial/ConvertToPhantomAction/hctConvertToPhantomActionFilter.h>

extern HINSTANCE hInstance;

INT_PTR CALLBACK _convertToPhantomActionDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{	
	// We store a pointer to the filter associated with this dialog using Get/SetWindowLongPtr() 
	hctConvertToPhantomActionFilter* filter = reinterpret_cast<hctConvertToPhantomActionFilter*> ( (hkUlong) GetWindowLongPtr(hWnd,GWLP_USERDATA)) ; 

	switch(message) 
	{
		case WM_INITDIALOG:
			{	
				filter = (hctConvertToPhantomActionFilter*)lParam;
				SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)lParam); // so that it can be retrieved later

				CheckDlgButton(hWnd, IDC_CB_RemoveMeshes, filter->m_options.m_removeMeshes);

				// Initialize Tool Tips
				{
					CreateToolTip(IDC_CB_RemoveMeshes, hWnd, hInstance, "If enabled, this option will remove the association of a rigid body node to its original mesh whenever it is converted to a phantom (so the phantom won't be displayed by the preview scene filter).");
				}

				return TRUE; // did handle it
			}
	}
	return FALSE; //didn't handle it
}	

void hctConvertToPhantomActionFilter::updateOptions()
{
	// Ensure the options we store match the options shown in the UI
	if (m_optionsDialog)
	{
		m_options.m_removeMeshes = IsDlgButtonChecked(m_optionsDialog, IDC_CB_RemoveMeshes) == TRUE;
	}
}


HWND hctConvertToPhantomActionFilter::showOptions(HWND owner)
{
	if (m_optionsDialog)
	{
		hideOptions();
	}

	m_optionsDialog = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_CONVERT_TO_PHANTOM_ACTION_DIALOG),
		owner, _convertToPhantomActionDialogProc, (LPARAM) this );

	return m_optionsDialog;
}

void hctConvertToPhantomActionFilter::hideOptions()
{
	// Update any changes before we close UI
	updateOptions();

	if (m_optionsDialog)
	{
		DestroyWindow(m_optionsDialog);
	}

	m_optionsDialog = NULL;

}

/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090216)
* 
* Confidential Information of Havok.  (C) Copyright 1999-2009
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
* 
* Use of this software for evaluation purposes is subject to and indicates
* acceptance of the End User licence Agreement for this product. A copy of
* the license is included with this software and is also available at www.havok.com/tryhavok.
* 
*/
