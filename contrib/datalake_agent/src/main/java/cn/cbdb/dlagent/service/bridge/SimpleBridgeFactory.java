package cn.cbdb.dlagent.service.bridge;

import cn.cbdb.dlagent.api.model.RequestContext;
import cn.cbdb.dlagent.service.utilities.BasePluginFactory;
import cn.cbdb.dlagent.service.utilities.GSSFailureHandler;
import org.springframework.stereotype.Component;

@Component
public class SimpleBridgeFactory implements BridgeFactory {

    private final BasePluginFactory pluginFactory;
    private final GSSFailureHandler failureHandler;

    public SimpleBridgeFactory(BasePluginFactory pluginFactory, GSSFailureHandler failureHandler) {
        this.pluginFactory = pluginFactory;
        this.failureHandler = failureHandler;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Bridge getBridge(RequestContext context) {
        return new ReadBridge(pluginFactory, context, failureHandler);
    }
}
