
pipeline {
    agent none
    stages {
        stage('initialize submodules') {
            agent {
                label 'generic'
            }
            steps {
                script {
                    sh 'git submodule update --init --recursive'
                }
            }
        }
        stage('Build') {
            matrix {
                axes {
                    axis {
                        name 'PLATFORM'
                        values /*'windows',*/ 'linux'/*, 'macos'*/
                    }
                    axis {
                        name 'COMPILER'
                        values 'gcc', 'clang'/*, 'msvc'*/
                    }
                    axis {
                        name 'BUILD_TYPE'
                        values 'Release', 'Debug'/*, 'MinSizeRel', 'RelWithDebInfo'*/
                    }
                }

                stages {
                    stage('Select Agent'){
                        agent {
                            label "${PLATFORM}&&${COMPILER}"
                        }
                        stages {
                            stage('Retrieving Artifacts') {
                                steps {
                                    script{
                                        try {
                                            unstash 'vendor'
                                        } catch(Exception e) {

                                        }
                                        try {
                                            unarchive (mapping: [
                                                "build-${PLATFORM}-${COMPILER}/": "build"
                                            ])
                                            artifactsRetrieved = true
                                        } catch (Exception e) {

                                        }
                                    }
                                }
                            }
                            stage('Prepare Tools'){
                                steps {
                                    script {
                                        if(COMPILER == 'clang') {
                                            sh 'apt install -y llvm'
                                        }
                                        sh '''
                                            apt install -y lcov doxygen python3-venv cppcheck cpplint
                                            python3 -m venv venv
                                            . venv/bin/activate
                                            pip install lcov_cobertura
                                        '''
                                    }
                                }
                            }
                            stage('Build') {
                                steps {
                                    sh """
                                    cmake -B build/ -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
                                    cmake --build build/ --config=${BUILD_TYPE} -j
                                    """
                                }
                            }
                            stage('Calculate Code Metrics') {
                                steps {
                                    catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
                                        sh """
                                        . venv/bin/activate
                                        cppcheck --enable=all --inconclusive --xml --xml-version=2 -i buiild/CMakeFiles -ibuild/_deps -ibuild/vendor build/ 2> cppcheck.xml
                                        cpplint --output=vs7 source/ include/ > cpplint.xml
                                        """
                                    }
                                }
                            }
                            stage('Creating Metric Report') {
                                steps {
                                    catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
                                        sh """
                                        cmake -B build/ -DCMAKE_BUILD_TYPE=Debug
                                        cmake --build build/ --config=${BUILD_TYPE} -j --target coverage

                                        . venv/bin/activate
                                        # Convert lcov report to cobertura format
                                        lcov_cobertura build/coverage.lcov -o coverage.xml
                                        """

                                        recordIssues(
                                            sourceCodeRetention: 'LAST_BUILD',
                                            tools: [
                                                cppCheck(id: "cppcheck_${PLATFORM}_${COMPILER}_${BUILD_TYPE}" ,pattern: 'cppcheck.xml'),
                                                cppLint(id: "cpplint_${PLATFORM}_${COMPILER}_${BUILD_TYPE}",pattern: 'cpplint.xml')
                                            ]
                                        )

                                        recordCoverage(
                                            qualityGates: [
                                                [criticality: 'NOTE', integerThreshold: 60, metric: 'FILE', threshold: 60.0],
                                                [criticality: 'NOTE', integerThreshold: 60, metric: 'LINE', threshold: 60.0],
                                                [criticality: 'NOTE', integerThreshold: 60, metric: 'METHOD', threshold: 60.0]
                                            ],
                                            tools: [[parser: 'COBERTURA', pattern: 'coverage.xml']])
                                    }
                                }
                            }
                            stage('Archiving Artifacts') {
                                steps {
                                    sh 'mv build "build-${PLATFORM}-${COMPILER}"'
                                    archiveArtifacts (artifacts: "build-${PLATFORM}-${COMPILER}/", allowEmptyArchive: true, onlyIfSuccessful: true, fingerprint: true)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
