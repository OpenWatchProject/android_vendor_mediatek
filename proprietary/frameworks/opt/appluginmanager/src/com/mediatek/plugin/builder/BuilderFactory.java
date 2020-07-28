package com.mediatek.plugin.builder;

import java.util.HashMap;

/**
 * Register all Builder for Parser XML.
 */
public class BuilderFactory {

    public static HashMap<String, Builder> sBuilders = new HashMap<String, Builder>();

    static {
        registerBuilder(new ParameterBuilder());
        registerBuilder(new ParameterDefBuilder());
        registerBuilder(new ExtensionBuilder());
        registerBuilder(new ExtensionPointBuilder());
        registerBuilder(new PluginDescriptorBuilder());
    }

    /**
     * Get the Builder by XML tag.
     * @param tag
     *            The XML tag.
     * @return the build for parser.
     */
    public static Builder getBuilder(String tag) {
        return sBuilders.get(tag);
    }

    private static void registerBuilder(Builder builder) {
        sBuilders.put(builder.getSupportedTag(), builder);
    }
}
