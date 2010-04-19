/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/
package com.nokia.cpix.support.results;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.LineNumberReader;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;

import com.nokia.cpix.util.FileIntroducer;
import com.nokia.cpix.util.Visitor;

public class PerformanceMetricsOutputParser implements Visitor<File> {
	
	public class Entry {
		public final Integer TIME; 
		public final Integer PEAK_MEMORY; 
		public Entry(Integer time, Integer peakMemory) {
			this.TIME = time; 
			this.PEAK_MEMORY = peakMemory; 
		}
	}
	
	private File input; 
	
	private File output; 
	
	public PerformanceMetricsOutputParser(File input, File output) {
		this.input = input; 
		this.output = output; 
	}
	
	public void run() {
		FileIntroducer.introduce(input, this);
	}
	
	public List<Entry> parse(InputStream in) throws IOException {
		LineNumberReader reader = new LineNumberReader(new InputStreamReader(in)); 
		try {
			for (int i = 0; i < 10; i++) {
				reader.readLine(); // skip
			}
			
			List<Entry> ret = new ArrayList<Entry>(); 
			
			while (true) {
				String hitcount = reader.readLine(); // likely hit count
				if (hitcount.contains("Index database size")) { // not hit count
					break; // interesting parts are over
				}
				String memory = reader.readLine(); // memory
				String peakMemory = reader.readLine(); // memory
				String time = reader.readLine(); // time
				
				int peakMemoryValue = Integer.parseInt(peakMemory.split(":")[2].trim());  
				int timeValue = Integer.parseInt(time.split(":")[1].replace("ms", "").trim()); 
				
				ret.add(new Entry(timeValue, peakMemoryValue)); 
			}
			return ret; 
		} finally {
			reader.close(); 
		}
	}

	public void visit(File t) {
		try {
			String relative = t.getAbsolutePath().substring(input.getAbsolutePath().length()); 
			File written = new File(output, relative.replace(".txt", ".csv"));
			written.getParentFile().mkdirs(); 
			Writer writer = 
				new OutputStreamWriter(
					new FileOutputStream(written));
			
			try {
				for (Entry e : parse(new FileInputStream(t))) {
					writer.write((""+((e.TIME/5)/1000f)).replace('.', ',') + "\n");
					writer.write((""+((e.PEAK_MEMORY)/1000000f)).replace('.', ',') + "\n");
				}
			} finally {
				writer.close();
			}
			
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

}
