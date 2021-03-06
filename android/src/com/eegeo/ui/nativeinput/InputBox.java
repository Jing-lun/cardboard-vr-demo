// Copyright eeGeo Ltd (2012-2014), All Rights Reserved

package com.eegeo.ui.nativeinput;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.text.InputType;
import android.util.Log;
import android.widget.EditText;

public class InputBox
{
	public static String text()
	{
		return ms_text;
	}

	private static String ms_text = "";
	private static AlertDialog m_inputBox = null;

	public native static void callback(long ptr);

	public static void close()
	{
		if(m_inputBox!=null)
		{
			m_inputBox.dismiss();
			m_inputBox = null;
		}
	}

	public static void popUpBox(
	    final Activity a,
	    final String title,
	    final String message,
	    final String button,
	    final String intitialContent,
	    final boolean initialContentIsPlaceholder,
	    final long ptr)
	{
		try
		{
			AlertDialog.Builder builder = new AlertDialog.Builder(a);
			builder.setTitle(title);
			builder.setMessage(message);

			final EditText input = new EditText(a);

			input.setInputType(InputType.TYPE_CLASS_TEXT);

			if(initialContentIsPlaceholder)
			{
				input.setHint(intitialContent);
			}
			else
			{
				input.setText(intitialContent);
			}

			builder.setView(input);
			builder.setCancelable(false);

			// Set up the buttons
			builder.setPositiveButton(button, new DialogInterface.OnClickListener()
			{
				@Override
				public void onClick(DialogInterface dialog, int which)
				{
					ms_text = input.getText().toString();
					callback(ptr);
					m_inputBox = null;
				}
			});

			m_inputBox = builder.show();
		}
		catch (Exception e)
		{
			Log.v("InputBox", e.getMessage() == null ? "Error, but no message?!" : e.getMessage());
		}
	}
}
