package com.mediatek.atmwifimeta;

/**
 * Created by MTK13237 on 2017/8/5.
 */

import android.content.Context;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 绫诲悕锛氳鍙栭厤缃被<br>
 * @author mtk13237
 *
 */
public class AtmConfigReader {

    /**
     * 鏁翠釜ini鐨勫紩鐢�
     */
    private Map<String,Map<String, List<String>>>  map = null;
    /**
     * 褰撳墠Section鐨勫紩鐢�
     */
    private String currentSection = null;


    /**
     * 璇诲彇
     * @param path
     */
    public AtmConfigReader(String path) {
        map = new HashMap<String, Map<String,List<String>>>();
        try {
            BufferedReader reader = new BufferedReader(new FileReader(path));
            read(reader);
        } catch (IOException e) {
            e.printStackTrace();
            throw new RuntimeException("IO Exception:" + e);
        }

    }

    /**
     * 璇诲彇鏂囦欢
     * @param reader
     * @throws IOException
     */
    private void read(BufferedReader reader) throws IOException {
        String line = null;
        while((line=reader.readLine())!=null) {
            parseLine(line);
        }
    }

    /**
     * 杞崲
     * @param line
     */
    private void parseLine(String line) {
        line = line.trim();
        // 姝ら儴鍒嗕负娉ㄩ噴
        if(line.matches("^\\#.*$")) {
            return;
        }else if (line.matches("^\\[\\S+\\]$")) {
            // section
            String section = line.replaceFirst("^\\[(\\S+)\\]$","$1");
            addSection(map,section);
        }else if (line.matches("^\\S+=.*$")) {
            // key ,value
            int i = line.indexOf("=");
            String key = line.substring(0, i).trim();
            String value =line.substring(i + 1).trim();
            addKeyValue(map,currentSection,key,value);
        }
    }


    /**
     * 澧炲姞鏂扮殑Key鍜孷alue
     * @param map
     * @param currentSection
     * @param key
     * @param value
     */
    private void addKeyValue(Map<String, Map<String, List<String>>> map,
                             String currentSection,String key, String value) {
        if(!map.containsKey(currentSection)) {
            return;
        }
        Map<String, List<String>> childMap = map.get(currentSection);

        if(!childMap.containsKey(key)) {
            List<String> list = new ArrayList<String>();
            list.add(value);
            childMap.put(key, list);
        } else {
            childMap.get(key).add(value);
        }
    }


    /**
     * 澧炲姞Section
     * @param map
     * @param section
     */
    private void addSection(Map<String, Map<String, List<String>>> map,
                            String section) {
        if (!map.containsKey(section)) {
            currentSection = section;
            Map<String,List<String>> childMap = new HashMap<String, List<String>>();
            map.put(section, childMap);
        }
    }

    /**
     * 鑾峰彇閰嶇疆鏂囦欢鎸囧畾Section鍜屾寚瀹氬瓙閿殑鍊�
     * @param section
     * @param key
     * @return
     */
    public List<String> get(String section,String key){
        if(map.containsKey(section)) {
            return  get(section).containsKey(key) ?
                    get(section).get(key): null;
        }
        return null;
    }



    /**
     * 鑾峰彇閰嶇疆鏂囦欢鎸囧畾Section鐨勫瓙閿拰鍊�
     * @param section
     * @return
     */
    public Map<String, List<String>> get(String section){
        return  map.containsKey(section) ? map.get(section) : null;
    }

    /**
     * 鑾峰彇杩欎釜閰嶇疆鏂囦欢鐨勮妭鐐瑰拰鍊�
     * @return
     */
    public Map<String, Map<String, List<String>>> get(){
        return map;
    }

    public Map<String,String> getSingleMap(String section){
        Map<String,String> singleMap = new HashMap<String,String>();
        Map<String, List<String>> compulxMap = get(section);
        String key = null;
        String value = null;
        if(compulxMap != null)
            for(Map.Entry<String,List<String>> entry :compulxMap.entrySet()){
                key = entry.getKey();
                value = entry.getValue() == null ? null : entry.getValue().get(0);
                singleMap.put(key,value);
            }
        return singleMap;

    }


}
