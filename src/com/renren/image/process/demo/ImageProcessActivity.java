package com.renren.image.process.demo;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.MediaStore;
import android.view.Menu;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

public class ImageProcessActivity extends Activity {
	private static final int REQUEST_CODE = 0x0001;

	Button chooseBT;
	TextView  logTV;
	ImageView originalIV;
	ImageView destinationIV;

	Handler handler = new Handler();

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		chooseBT = (Button) findViewById(R.id.bt_choose);
		originalIV = (ImageView) findViewById(R.id.iv_original);
		destinationIV = (ImageView) findViewById(R.id.iv_destination);
		logTV = (TextView) findViewById(R.id.tv_log);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	public void chooseImage(View v) {
		Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
		intent.setType("image/*");
		startActivityForResult(intent, REQUEST_CODE);
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);

		if (requestCode == REQUEST_CODE && resultCode == RESULT_OK) {
			Uri uri = data.getData();

			String[] project = { MediaStore.Images.Media.DATA };
			Cursor cursor = managedQuery(uri, project, null, null, null);
			int index = cursor
					.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
			cursor.moveToFirst();
			final String fileFullPath = cursor.getString(index);

			handler.post(new Runnable() {
				@Override
				public void run() {
					long start = System.currentTimeMillis();
					processImage(fileFullPath, getOutFilePath(fileFullPath));
					long end = System.currentTimeMillis();
					System.out.println("process time is: " + (end - start));

					Toast.makeText(ImageProcessActivity.this,
							"Process success!", Toast.LENGTH_SHORT).show();
					logTV.setText("generate file '" +  getOutFilePath(fileFullPath) + "'");
				}
			});
		}
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
	}

	private String getOutFilePath(String fileFullPath) {
		int fileIndex = fileFullPath.lastIndexOf(".");

		return fileFullPath.substring(0, fileIndex) + "_out"
				+ fileFullPath.substring(fileIndex);
	}

	public native String testLib(String libName);

	public native void processImage(String inFilePath, String outFilePath);

	static {
		System.loadLibrary("image");
	}

}
