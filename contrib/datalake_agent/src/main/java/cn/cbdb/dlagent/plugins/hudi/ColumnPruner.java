package cn.cbdb.dlagent.plugins.hudi;

import cn.cbdb.dlagent.api.filter.ColumnIndexOperandNode;
import cn.cbdb.dlagent.api.filter.Node;
import cn.cbdb.dlagent.api.filter.Operator;
import cn.cbdb.dlagent.api.filter.OperatorNode;
import cn.cbdb.dlagent.api.filter.SupportedOperatorPruner;
import cn.cbdb.dlagent.api.utilities.ColumnDescriptor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.EnumSet;
import java.util.List;
import java.util.Set;

/**
 * Prune the tree based on partition keys
 */
public class ColumnPruner extends SupportedOperatorPruner {

    private static final Logger LOG = LoggerFactory.getLogger(ColumnPruner.class);

    private final Set<String> columns;
    private final List<ColumnDescriptor> columnDescriptors;
    private final boolean isIncludeMode;

    public ColumnPruner(EnumSet<Operator> supportedOperators,
                        Set<String> columns,
                        List<ColumnDescriptor> columnDescriptors,
                        boolean isIncludeMode) {
        super(supportedOperators);
        this.columns = columns;
        this.columnDescriptors = columnDescriptors;
        this.isIncludeMode = isIncludeMode;
    }

    @Override
    public Node visit(Node node, final int level) {
        if (node instanceof OperatorNode &&
                !shouldKeep((OperatorNode) node)) {
            return null;
        }
        return super.visit(node, level);
    }

    /**
     * Returns true when the operatorNode is logical, or for simple operators
     * true when the column is a partitioned column
     *
     * @param operatorNode the operatorNode node
     * @return true when the filter is compatible, false otherwise
     */
    private boolean shouldKeep(OperatorNode operatorNode) {
        Operator operator = operatorNode.getOperator();

        if (operator.isLogical()) {
            // Skip AND / OR
            return true;
        }

        ColumnIndexOperandNode columnIndexOperand = operatorNode.getColumnIndexOperand();
        ColumnDescriptor columnDescriptor = columnDescriptors.get(columnIndexOperand.index());
        String columnName = columnDescriptor.columnName();

        boolean found = columns.contains(columnName);
        if (isIncludeMode) {
            return found;
        }

        return !found;
    }
}
